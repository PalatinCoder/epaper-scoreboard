#include "GattSpecial.h"

using namespace BLE::Special;

UserDescriptionDescriptor::UserDescriptionDescriptor(std::string value) : Descriptor(BLE::UUID(0x2901)), m_value(value) {}
UserDescriptionDescriptor::~UserDescriptionDescriptor() {}
void* UserDescriptionDescriptor::GetValue() { return (void*)m_value.c_str(); }
size_t UserDescriptionDescriptor::GetValueSize() { return m_value.length(); }