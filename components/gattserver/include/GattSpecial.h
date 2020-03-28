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
            static BLE::UUID* m_uuid;
            int m_value;
    };

    class BatteryLevelService : public Service {
        public:
            BatteryLevelService();
            ~BatteryLevelService();
        private:
            static BLE::UUID* m_uuid;
            BatteryLevelCharacteristic* chr;
    };

} // namespace BLE::Special

#endif // #ifndef _GATT_SERVER_SPECIAL_H