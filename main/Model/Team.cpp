#include "Scoreboard.h"
#include "esp_log.h"

using namespace Model;

static const char* TAG = "Model::Team";

Team::Team(BLE::UUID uuid, std::string desc, std::string name, INotifyModelChanged cb) : BLE::Characteristic(BLE::UUID(uuid), BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE), m_name(name), notifyModelChanged(cb) {
    this->m_desc = new BLE::Special::UserDescriptionDescriptor(desc);
    this->AddDescriptor(m_desc);
}
void Team::setName(std::string name) { this->m_name = name; notifyModelChanged(); }
std::string Team::getName() { return this->m_name; }

void* Team::GetValue() { return (void*)m_name.c_str(); }
size_t Team::GetValueSize() { return m_name.length(); }

void Team::SetValueFromBuffer(void* buf, uint16_t len) {
    this->setName(std::string((char*)buf, len));
}
uint16_t Team::value_maxlen() { return 50; }

Team::operator std::__cxx11::string() { return m_name; }