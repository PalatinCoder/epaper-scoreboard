#include "Scoreboard.h"

using namespace Model;

Score::Score(BLE::UUID uuid, std::string description) : BLE::Characteristic(BLE::UUID(uuid), BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE), m_score(0) {
    this->m_desc = new BLE::Special::UserDescriptionDescriptor(description);
    this->AddDescriptor(m_desc);
}
void Score::setScore(uint8_t score) { this->m_score = score; }
uint8_t Score::getScore() { return this->m_score; }

void* Score::GetValue() { return (void*)&m_score; }
size_t Score::GetValueSize() { return sizeof(m_score); }

uint16_t Score::value_maxlen() { return 1; }
void Score::SetValueFromBuffer(void* buf, uint16_t len) {
    /* buffer is guaranteed by the low level implementation to be 1 byte only at this point, as we have set value_maxlen to 1 */
    this->setScore(*(uint8_t*)buf);
}
