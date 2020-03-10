#include "GattServer.h"

using namespace BLE;

UUID::UUID(uint16_t uuid) {
    ESP_ERROR_CHECK(ble_uuid_init_from_buf(&m_uuid, &uuid, 2));
}

UUID::UUID(uint8_t* bytes) {
    ESP_ERROR_CHECK(ble_uuid_init_from_buf(&m_uuid, bytes, 16));
}

UUID::operator ble_uuid_t* () { return (ble_uuid_t*)&m_uuid; }