#ifndef _GATT_SERVER_H
#define _GATT_SERVER_H

#include <vector>
#include "host/ble_uuid.h"
#include "host/ble_hs.h"
#include "host/ble_gatt.h"

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


    class GattServer {
    private:
        std::vector<ble_gatt_svc_def> m_services;
        int GAPEventHandler(ble_gap_event* event, void* arg);
        void Advertise();
    public:
        static GattServer& Instance();
        ~GattServer() {}
        void Run();
        void Init(const char* name);
        void AddService(Service* s);
    protected:
        GattServer() {};
        GattServer(const GattServer& other) {};
        GattServer& operator = (const GattServer&);
    };
} // namespace BLE
#endif // _GATT_SERVER_H