#include "GattServer.h"
#include "esp_log.h"

static const char* TAG = "BLE::Descriptor";

using namespace BLE;

Descriptor::Descriptor(BLE::UUID uuid) {
    m_uuid = new BLE::UUID(uuid);

    m_dsc_def.access_cb = Descriptor::AccessHandler;
    m_dsc_def.arg = this; // pass this object to the access callback, so it has access to the object members
    m_dsc_def.att_flags = 0x01; // no idea.. read only?
    m_dsc_def.min_key_size = 0;
    m_dsc_def.uuid = *m_uuid;

    ESP_LOGD(TAG, "Initialized descriptor object");
}

Descriptor::~Descriptor() {
    delete m_uuid;
    ESP_LOGD(TAG, "Deleted descriptor object");
}

int Descriptor::AccessHandler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    Descriptor* self = (Descriptor*)arg;

    switch(ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_DSC:
        ESP_LOGD(TAG, "Read access to descriptor");
        return os_mbuf_append(ctxt->om, self->GetValue(), self->GetValueSize()) == 0 ? 0: BLE_ATT_ERR_INSUFFICIENT_RES;
    default:
        return BLE_ATT_ERR_UNLIKELY;
    }
}

Descriptor::operator ble_gatt_dsc_def() {
    return m_dsc_def;
}

/* Generalized class doesn't hold a value */
void* Descriptor::GetValue() { return nullptr; }
size_t Descriptor::GetValueSize() { return 0; }