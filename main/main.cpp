#include "esp_log.h"
#include "Scoreboard.h"

static const char* TAG = "main";

/**
 * What still should be done here:
 * 
 * The callback game isn't really C++ like
 * Updating the view should be handled by a seperate thread, to not block the BLE response
 * Is this thing thread safe anyways?...
 */

Model::Score* scoreHome;
Model::Score* scoreAway;
Model::Team* teamHome;
Model::Team* teamAway;

BLE::Service* scoreboard;
BLE::Service* battery;

static void UpdateView() {
    View::Instance().Update(*scoreHome, *scoreAway, *teamHome, *teamAway);
}

extern "C" void app_main()
{
    ESP_LOGI(TAG, "initialized");

    /* All our UUIDs */
    uint8_t serviceUuid[16]   = {0x50, 0x6a, 0x35, 0x36, 0x38, 0xee, 0xb5, 0xb7, 0x8e, 0xef, 0x13, 0xf8, 0xca, 0xbe, 0xca, 0x2f};
    uint8_t scoreHomeUuid[16] = {0x51, 0x6a, 0x35, 0x36, 0x38, 0xee, 0xb5, 0xb7, 0x8e, 0xef, 0x13, 0xf8, 0xca, 0xbe, 0xca, 0x2f};
    uint8_t scoreAwayUuid[16] = {0x52, 0x6a, 0x35, 0x36, 0x38, 0xee, 0xb5, 0xb7, 0x8e, 0xef, 0x13, 0xf8, 0xca, 0xbe, 0xca, 0x2f};
    uint8_t teamHomeUuid[16]  = {0x53, 0x6a, 0x35, 0x36, 0x38, 0xee, 0xb5, 0xb7, 0x8e, 0xef, 0x13, 0xf8, 0xca, 0xbe, 0xca, 0x2f};
    uint8_t teamAwayUuid[16]  = {0x54, 0x6a, 0x35, 0x36, 0x38, 0xee, 0xb5, 0xb7, 0x8e, 0xef, 0x13, 0xf8, 0xca, 0xbe, 0xca, 0x2f};

    /* Setup our data models (which are also the BLE characteristics) */
    scoreHome = new Model::Score(BLE::UUID(scoreHomeUuid), "Punkte Heim", UpdateView);
    scoreAway = new Model::Score(BLE::UUID(scoreAwayUuid), "Punkte Gast", UpdateView);
    teamHome  = new Model::Team (BLE::UUID(teamHomeUuid),  "Heimmannschaft", "VfL Duttweiler", UpdateView);
    teamAway  = new Model::Team (BLE::UUID(teamAwayUuid),  "Gastmannschaft", "Teamname", UpdateView);

    /* Setup the service */
    scoreboard = new BLE::Service(BLE::UUID(serviceUuid), BLE_GATT_SVC_TYPE_PRIMARY);
    scoreboard->AddCharacteristic(scoreHome);
    scoreboard->AddCharacteristic(scoreAway);
    scoreboard->AddCharacteristic(teamHome);
    scoreboard->AddCharacteristic(teamAway);

    /* Setup the battery service, as a dummy for now */
    battery = new BLE::Special::BatteryLevelService([]{ return (uint8_t)100; });

    /* Fire up the server */
    BLE::GattServer::Instance().Init("Scoreboard");

    /* Wire up the serivces to the server */
    BLE::GattServer::Instance().AddService(scoreboard);
    BLE::GattServer::Instance().AddService(battery);

    /* Setup the View */
    View::Instance().Init();
    View::Instance().Standby();

    /* Let's go! */
    BLE::GattServer::Instance().Run();
}
