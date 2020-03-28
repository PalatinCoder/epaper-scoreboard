#include <cstring>
#include "GattServer.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/util/util.h"
#include "esp_log.h"
#include "nvs_flash.h"

using namespace BLE;

static const char* TAG = "BLE::GattServer";

GattServer& GattServer::Instance() {
    static GattServer instance;
    return instance;
}

void GattServer::Init(const char* name = "NIMBLE") {

    /* Init nvs, needed by NimBLE to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    // if full, clear and try again
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Init NimBLE Stack */
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();

    // dummy reset callback
    //ble_hs_cfg.reset_cb = [](int reason) { ESP_LOGE(TAG, "Resetting state; reason=%d", reason); };

    // Sync callback get's called when the BLE stack is ready. We start advertising then
    ble_hs_cfg.sync_cb = [] { Instance().Advertise(); };

    // Q&D logger for registration events
    ble_hs_cfg.gatts_register_cb = [](ble_gatt_register_ctxt* ctxt, void* arg) {
        char buf[BLE_UUID_STR_LEN];
        switch(ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            ESP_LOGD(TAG, "Registering service %s", ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf));
            break;
        case BLE_GATT_REGISTER_OP_CHR:
            ESP_LOGD(TAG, "Registering characteristic %s", ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf));
            break;
        case BLE_GATT_REGISTER_OP_DSC:
            ESP_LOGD(TAG, "Registering descriptor %s", ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf));
            break;
        default:
            assert(0);
            break;
        }
    };

    // we have no I/O capability
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    
    /* Set our name */
    ESP_ERROR_CHECK(ble_svc_gap_device_name_set(name));

    /* Init common GAP and GATT services, handled by the BLE stack */
    ble_svc_gap_init();
    ble_svc_gatt_init();

    /* Now we're ready to roll */
    ESP_LOGI(TAG, "initialized and ready");
}

void GattServer::Run() {
    /* Register all the services before we start advertising */
    ESP_LOGD(TAG, "Server has %d services", m_services.size());
    m_services.push_back((ble_gatt_svc_def) { BLE_GATT_SVC_TYPE_END });
    ESP_ERROR_CHECK(ble_gatts_count_cfg(&m_services[0]));
    ESP_ERROR_CHECK(ble_gatts_add_svcs(&m_services[0]));
 
    nimble_port_freertos_init([] (void* param) {
        ESP_LOGI(TAG, "task started");
        nimble_port_run(); // <-- this call returns when ble is stopped
        nimble_port_freertos_deinit();
    });
}

void GattServer::Advertise() {
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;

    memset(&fields, 0, sizeof fields);
    memset(&adv_params, 0, sizeof adv_params);

    // Advertise as 1) general discoverable 2) ble only
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    // include tx power in advertisment and have it automatically filled
    fields.tx_pwr_lvl_is_present = true;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    const char* name = ble_svc_gap_device_name();
    fields.name = (uint8_t*)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = true;
    // TODO: Include service ids in advertising packet fields?

    ESP_ERROR_CHECK(ble_gap_adv_set_fields(&fields));

    // Advertise as generally discoverable and undirected connection
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ESP_ERROR_CHECK(ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, [](ble_gap_event* e, void* a) -> int { return Instance().GAPEventHandler(e,a); }, NULL));
    ESP_LOGI(TAG, "is advertising");
}

int GattServer::GAPEventHandler(ble_gap_event* event, void* arg) {
    //ble_gap_conn_desc ConnectionDescriptor;

    switch(event->type) {
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI(TAG, "connection %s; status=%d", event->connect.status == 0 ? "established" : "failed", event->connect.status);
            //TODO LOG connection
            /* connection failed, resume advertising */
            if(event->connect.status != 0) this->Advertise();
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "disconnect; reason=%d", event->disconnect.reason);
            // TODO LOG connection
            this->Advertise();
            break;
        case BLE_GAP_EVENT_CONN_UPDATE:
            ESP_LOGI(TAG, "connection updated; status=%d", event->conn_update.status);
            break;
        case BLE_GAP_EVENT_MTU:
            ESP_LOGI(TAG, "mtu update event; conn_handle=%d cid=%d mtu=%d", event->mtu.conn_handle, event->mtu.channel_id, event->mtu.value);
            break;
        case BLE_GAP_EVENT_SUBSCRIBE:
            ESP_LOGI(TAG, "subscribe event");
            break;
    }
    return 0;
}

void GattServer::AddService(Service* s) {
    m_services.push_back(*s);
}

