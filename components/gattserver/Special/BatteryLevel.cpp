
#include "GattSpecial.h"

using namespace BLE::Special;

BatteryLevelService::BatteryLevelService() : Service(BLE::UUID(0x180F), BLE_GATT_SVC_TYPE_PRIMARY) {
    chr = new BatteryLevelCharacteristic();
    this->AddCharacteristic(chr);
}
BatteryLevelService::~BatteryLevelService() { delete chr; }

BatteryLevelCharacteristic::BatteryLevelCharacteristic() : Characteristic(BLE::UUID(0x2A19), BLE_GATT_CHR_F_READ) { m_value = 25; };
void* BatteryLevelCharacteristic::GetValue() { return &m_value; }
size_t BatteryLevelCharacteristic::GetValueSize() { return sizeof(m_value); }