
#include "GattSpecial.h"

using namespace BLE::Special;

BatteryLevelService::BatteryLevelService(BatteryLevelCharacteristic::BatteryLevelCallback cb) : Service(BLE::UUID(0x180F), BLE_GATT_SVC_TYPE_PRIMARY) {
    chr = new BatteryLevelCharacteristic(cb);
    this->AddCharacteristic(chr);
}
BatteryLevelService::~BatteryLevelService() { delete chr; }

BatteryLevelCharacteristic::BatteryLevelCharacteristic(BatteryLevelCharacteristic::BatteryLevelCallback cb) : Characteristic(BLE::UUID(0x2A19), BLE_GATT_CHR_F_READ), batteryCallback(cb), m_value(0) {}
void* BatteryLevelCharacteristic::GetValue() { 
    if (batteryCallback != nullptr) m_value = batteryCallback();
    return &m_value; 
}

size_t BatteryLevelCharacteristic::GetValueSize() { return sizeof(m_value); }