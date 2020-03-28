
#include "GattSpecial.h"

using namespace BLE::Special;

BLE::UUID* BatteryLevelService::m_uuid = new BLE::UUID(0x180F);
BatteryLevelService::BatteryLevelService() : Service(m_uuid, BLE_GATT_SVC_TYPE_PRIMARY) {
    chr = new BatteryLevelCharacteristic();
    this->AddCharacteristic(chr);
}
BatteryLevelService::~BatteryLevelService() { delete chr; }

BLE::UUID* BatteryLevelCharacteristic::m_uuid = new BLE::UUID(0x2A19);
BatteryLevelCharacteristic::BatteryLevelCharacteristic() : Characteristic(m_uuid, BLE_GATT_CHR_F_READ) { m_value = 50; };
void* BatteryLevelCharacteristic::GetValue() { return &m_value; }
size_t BatteryLevelCharacteristic::GetValueSize() { return sizeof(m_value); }