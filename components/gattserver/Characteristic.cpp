#include "GattServer.h"

static const char* TAG = "BLE::Characteristic";

using namespace BLE;

Characteristic::Characteristic(BLE::UUID* uuid, ble_gatt_chr_flags flags) {
    m_chr_def.uuid = *uuid;
    m_chr_def.flags = flags;
    m_chr_def.access_cb = Characteristic::AccessHandler;
    m_chr_def.arg = this;
    m_chr_def.descriptors = NULL; // for now
    m_chr_def.min_key_size = 0;
    m_chr_def.val_handle = NULL;
    ESP_LOGD(TAG, "Initialized characteristic object");
}

Characteristic::~Characteristic() { ESP_LOGD(TAG, "Deleted characteristic object"); }

int Characteristic::AccessHandler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    Characteristic* self = (Characteristic*)arg;

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        ESP_LOGD(TAG, "Read Access to characteristic");
        return os_mbuf_append(ctxt->om, self->GetValue(), self->GetValueSize()) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        ESP_LOGD(TAG, "Write Access to characteristic");
        return self->SetValue(ctxt->om);
    default:
        return BLE_ATT_ERR_UNLIKELY;
    }
}

Characteristic::operator ble_gatt_chr_def() { return m_chr_def; }

/* Generalized class can't return a value, these methods must be overriden by specialized class */
void* Characteristic::GetValue() { return nullptr; }
size_t Characteristic::GetValueSize() { return 0; }
int Characteristic::SetValue(os_mbuf* om) { return BLE_ATT_ERR_UNLIKELY; }

