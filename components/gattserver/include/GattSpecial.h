#ifndef _GATT_SERVER_SPECIAL_H
#define _GATT_SERVER_SPECIAL_H

/* Specialized derived classes for certain use cases */

#include "GattServer.h"

namespace BLE::Special {

    class BatteryLevelCharacteristic : public Characteristic {
        public:
            BatteryLevelCharacteristic();
            void* GetValue() override;
            size_t GetValueSize() override;
        private:
            int m_value;
    };

    class BatteryLevelService : public Service {
        public:
            BatteryLevelService();
            ~BatteryLevelService();
        private:
            BatteryLevelCharacteristic* chr;
    };

    class UserDescriptionDescriptor : public BLE::Descriptor {
    private:
        std::string m_value;
    public:
        UserDescriptionDescriptor(std::string value);
        virtual ~UserDescriptionDescriptor();
        void* GetValue() override;
        size_t GetValueSize() override;
    };

} // namespace BLE::Special

#endif // #ifndef _GATT_SERVER_SPECIAL_H