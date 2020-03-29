#include "GattServer.h"

static const char* TAG = "BLE::Characteristic";

using namespace BLE;

Characteristic::Characteristic(BLE::UUID uuid, ble_gatt_chr_flags flags) {
    /* Copy the UUID into the object */
    m_uuid = new BLE::UUID(uuid);

    m_chr_def.uuid = *m_uuid;
    m_chr_def.flags = flags;
    m_chr_def.access_cb = Characteristic::AccessHandler;
    m_chr_def.arg = this;
    m_chr_def.descriptors = NULL; // for now
    m_chr_def.min_key_size = 0;
    m_chr_def.val_handle = NULL;

    ESP_LOGD(TAG, "Initialized characteristic object");
}

Characteristic::~Characteristic() { 
    delete m_uuid;
    ESP_LOGD(TAG, "Deleted characteristic object"); 
}

void Characteristic::AddDescriptor(Descriptor* d) {
    this->m_descriptors.push_back(*d);
}

int Characteristic::AccessHandler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    Characteristic* self = (Characteristic*)arg;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        ESP_LOGD(TAG, "Read Access to characteristic");
        return os_mbuf_append(ctxt->om, self->GetValue(), self->GetValueSize()) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
    {
        ESP_LOGD(TAG, "Write Access to characteristic");

        uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
        if (om_len < 1 || om_len > self->value_maxlen()) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

        /* hold the written value in a local buffer and pass it to the higher level handler */
        char* buf = new char[om_len];
        uint16_t len; // bytes written to buffer
        if (ble_hs_mbuf_to_flat(ctxt->om, buf, om_len, &len) > 0) return BLE_ATT_ERR_UNLIKELY;
        self->SetValueFromBuffer(buf, len);
        delete buf; // avoid memory leak

        return 0;
    }

    default:
        return BLE_ATT_ERR_UNLIKELY;
    }
}

Characteristic::operator ble_gatt_chr_def() { 
    /* finalize the list of descriptors */
    m_descriptors.push_back((ble_gatt_dsc_def) { NULL });
    m_chr_def.descriptors = &(m_descriptors[0]);

    return m_chr_def;
}

/* Generalized class can't return a value, these methods must be overriden by specialized class */
void* Characteristic::GetValue() { return nullptr; }
size_t Characteristic::GetValueSize() { return 0; }
uint16_t Characteristic::value_maxlen() { return 0; }
void Characteristic::SetValueFromBuffer(void* buf, uint16_t len) { return; }
