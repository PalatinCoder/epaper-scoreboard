#include "GattServer.h"
#include "esp_log.h"

static const char* TAG = "BLE::Service";

using namespace BLE;

Service::Service(BLE::UUID uuid, uint8_t type) { 
    /* Copy the uuid into this object*/
    m_uuid = new BLE::UUID(uuid);

    m_svc_def.type = type;
    m_svc_def.uuid = *m_uuid;
    m_svc_def.includes = NULL; // no includes yet
    m_svc_def.characteristics = NULL; // for the moment

    ESP_LOGD(TAG, "initialized service object");
}

Service::~Service() { 
    delete m_uuid;
    ESP_LOGD(TAG, "deleted service object"); 
}

void Service::AddCharacteristic(Characteristic* c) {
    m_characteristics.push_back(*c);
}

Service::operator ble_gatt_svc_def() { 
    m_characteristics.push_back((ble_gatt_chr_def) { NULL });
    m_svc_def.characteristics = &(m_characteristics[0]);
    return m_svc_def; 
}
