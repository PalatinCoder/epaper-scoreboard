#include "Scoreboard.h"
#include "esp_log.h"
#include "Logo.cpp"

static const char* TAG = "View";

View& View::Instance() {
    static View instance;
    return instance;
}

void View::Init() {
    this->displayConfig = {
        .busy_pin = 25,
        .reset_pin = 26,
        .rst_active_level = 0,
        .dc_pin = 27,
        .dc_level_data = 1,
        .dc_level_cmd = 0,
        .cs_pin = 15,
        .mosi_pin = 14,
        .miso_pin = -1, // unconnected
        .sck_pin = 13,
        .clk_freq_hz = 20* 1000 * 1000,
        .width = 176,
        .height = 264,
        .color_inv = false
    };

    this->Standby();
}

void View::Standby() {
    ESP_LOGI(TAG, "Setting screensaver");

    EPaper::Display display(&displayConfig);

    display.Rotate(EPaper::E_PAPER_ROTATE_90);

    display.ClearFramebuffer();

    auto pixelPtr = logo;

    int logoSize = 150;
    int centerX = display.CanvasWidth() * 0.5;
    int centerY = display.CanvasHeight() * 0.5;
    int RefX = centerX - logoSize * 0.5;
    int RefY = centerY - logoSize * 0.5;

    int row,col;
    for (row = 0; row < logoSize; row++) {
        for (col = 0; col < logoSize; col++) {
            if (*pixelPtr & (0x80 >> (col % 8))) {
                display.drawPixel(RefX + col, RefY + row);
            }
            if (col % 8 == 7) pixelPtr++;
        }
        if (logoSize % 8 != 0) pixelPtr++;
    }

    display.ShowFramebuffer();
}

void View::Update(std::string scoreHome, std::string scoreAway, std::string teamHome, std::string teamAway) {
    ESP_LOGI(TAG, "Updating View %s - %s \t\t %s : %s", teamHome.c_str(), teamAway.c_str(), scoreHome.c_str(), scoreAway.c_str());

    EPaper::Display display(&displayConfig);
    EPaper::Font::CourierNew16pt teamNameFont;
    EPaper::Font::CourierNew24pt scoreFont;
    EPaper::Text homeTeam(teamHome, &teamNameFont, EPaper::Text::ALIGN_LEFT);
    EPaper::Text guestTeam(teamAway, &teamNameFont, EPaper::Text::ALIGN_RIGHT);
    EPaper::Text homeScore(scoreHome, &scoreFont, EPaper::Text::ALIGN_CENTER);
    EPaper::Text guestScore(scoreAway, &scoreFont, EPaper::Text::ALIGN_CENTER);
    EPaper::Text divider(":", &scoreFont, EPaper::Text::ALIGN_CENTER);

    display.Rotate(EPaper::E_PAPER_ROTATE_90);

    display.ClearFramebuffer();


    /* Scores */
    display.drawText(&homeScore, display.CanvasWidth() * 0.25, display.CanvasHeight() * 0.5);
    display.drawText(&divider, display.CanvasWidth() * 0.5, display.CanvasHeight() * 0.5);
    display.drawText(&guestScore, display.CanvasWidth() * 0.75, display.CanvasHeight() * 0.5);

    /* Teamname Backgroundbox */
    display.drawRectangle(0, 0, display.CanvasWidth(), homeTeam.BoxHeight() + guestTeam.BoxHeight(), true);
    display.Invert();
    display.drawText(&homeTeam, 10, 0);
    display.drawText(&guestTeam, display.CanvasWidth() - 10, homeTeam.BoxHeight());

    display.ShowFramebuffer();
}

