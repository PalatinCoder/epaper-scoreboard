#ifndef _GATT_SERVER_H
#define _GATT_SERVER_H

#include <vector>
#include <string>
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

    /**
     * General BLE descriptor class. This class should not be constructed directly but be subclassed instead
     */
    class Descriptor {
    private:
        BLE::UUID* m_uuid;
        ble_gatt_dsc_def m_dsc_def;
        /**
         * Callback for the NimBLE stack to handle access to a descriptor
         * @param arg This should be set to the 'this' pointer of the object, so the access handler has access to the instance methods
         */
        static int AccessHandler(uint16_t conn_handle, uint16_t attr_handle, ble_gatt_access_ctxt* ctxt, void* arg);
    protected:
        /**
         * This is called by the access handler to get the value of the descriptor. Derived classes MUST override this
         */
        virtual void* GetValue();
        /**
         * Get the size of the descriptor's value. Access handler needs to know the size to append to the output buffer. Derived classes MUST override this
         */
        virtual size_t GetValueSize();
    public:
        Descriptor(BLE::UUID uuid);
        virtual ~Descriptor();
        operator ble_gatt_dsc_def();
    };

    /**
     * General BLE Characteristic class. This class should not be used directly but be subclassed instead
     */
    class Characteristic {
    private:
        std::vector<ble_gatt_dsc_def> m_descriptors;
        ble_gatt_chr_def m_chr_def;
        BLE::UUID* m_uuid;
        /**
         * Callback for the NimBLE stack to handle access to a descriptor
         * @param arg This should be set to the 'this' pointer of the object, so the access handler has access to the instance methods
         */
        static int AccessHandler(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg);
    protected:
        /**
         * This is called by the access handler to get the value of the characteristic. Derived classes MUST override this
         */
        virtual void* GetValue();
        /**
         * Get the size of the characteristic's value. Access handler needs to know the size to append to the output buffer. Derived classes MUST override this
         */
        virtual size_t GetValueSize();
        /**
         * Set the characteristic's value from a buffer. Derived classes MUST override this, if they are writeable
         */
        virtual void SetValueFromBuffer(void* buf, uint16_t len);
        /**
         * return the max num of bytes that can be written to the characteristic's value. Derived classes MUST override this, if they are writeable
         */
        virtual uint16_t value_maxlen();
    public:
        /**
         * Add a descriptor to the characteristic. Caller must make sure that the descriptor object lives at least as long as the characteristic lives
         */
        void AddDescriptor(Descriptor* d);
        Characteristic(BLE::UUID uuid, ble_gatt_chr_flags flags);
        virtual ~Characteristic();
        operator ble_gatt_chr_def();
    };

    /**
     * General BLE Service
     */
    class Service {
    private:
        std::vector<ble_gatt_chr_def> m_characteristics;
        ble_gatt_svc_def m_svc_def;
        BLE::UUID* m_uuid;
    public:
        /**
         * Add a characteristic to the service. Make sure the characteristic object lives as long as the service
         */
        void AddCharacteristic(Characteristic* c);
        Service(BLE::UUID uuid, uint8_t type);
        virtual ~Service();
        operator ble_gatt_svc_def();
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