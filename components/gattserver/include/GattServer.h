#ifndef _GATT_SERVER_H
#define _GATT_SERVER_H

#include "host/ble_uuid.h"

namespace BLE {

    class UUID {
    private:
        ble_uuid_any_t m_uuid;
    public:
        /**
         * Create a 16bit well-known UUID
         */
        UUID(uint16_t value);
        /**
         * Create a 128bit custom UUID
         */
        UUID(uint8_t* value);
        /**
         * Allow an instance of this class to be cast to the target type for the NimBLE API 
         */
        operator ble_uuid_t*();
    };

} // namespace BLE
#endif // _GATT_SERVER_H