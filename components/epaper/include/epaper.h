#ifndef _EPAPER_DRIVER_H
#define _EPAPER_DRIVER_H

#include <driver/spi_master.h>
#include <driver/gpio.h>

#define EPAPER_CS_SETUP_NS      55
#define EPAPER_CS_HOLD_NS       60
#define EPAPER_1S_NS            1000000000
#define EPAPER_QUEUE_SIZE_DEFAULT 10

#define COLORED         0
#define UNCOLORED       1
// epd2in7 commands
#define E_PAPER_PANEL_SETTING                               0x00
#define E_PAPER_POWER_SETTING                               0x01
#define E_PAPER_POWER_OFF                                   0x02
#define E_PAPER_POWER_OFF_SEQUENCE_SETTING                  0x03
#define E_PAPER_POWER_ON                                    0x04
#define E_PAPER_POWER_ON_MEASURE                            0x05
#define E_PAPER_BOOSTER_SOFT_START                          0x06
#define E_PAPER_DEEP_SLEEP                                  0x07
#define E_PAPER_DATA_START_TRANSMISSION_1                   0x10
#define E_PAPER_DATA_STOP                                   0x11
#define E_PAPER_DISPLAY_REFRESH                             0x12
#define E_PAPER_DATA_START_TRANSMISSION_2                   0x13
#define E_PAPER_PARTIAL_DATA_START_TRANSMISSION_1           0x14
#define E_PAPER_PARTIAL_DATA_START_TRANSMISSION_2           0x15
#define E_PAPER_PARTIAL_DISPLAY_REFRESH                     0x16
#define E_PAPER_LUT_FOR_VCOM                                0x20
#define E_PAPER_LUT_WHITE_TO_WHITE                          0x21
#define E_PAPER_LUT_BLACK_TO_WHITE                          0x22
#define E_PAPER_LUT_WHITE_TO_BLACK                          0x23
#define E_PAPER_LUT_BLACK_TO_BLACK                          0x24
#define E_PAPER_PLL_CONTROL                                 0x30
#define E_PAPER_TEMPERATURE_SENSOR_COMMAND                  0x40
#define E_PAPER_TEMPERATURE_SENSOR_CALIBRATION              0x41
#define E_PAPER_TEMPERATURE_SENSOR_WRITE                    0x42
#define E_PAPER_TEMPERATURE_SENSOR_READ                     0x43
#define E_PAPER_VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define E_PAPER_LOW_POWER_DETECTION                         0x51
#define E_PAPER_TCON_SETTING                                0x60
#define E_PAPER_TCON_RESOLUTION                             0x61
#define E_PAPER_SOURCE_AND_GATE_START_SETTING               0x62
#define E_PAPER_GET_STATUS                                  0x71
#define E_PAPER_AUTO_MEASURE_VCOM                           0x80
#define E_PAPER_VCOM_VALUE                                  0x81
#define E_PAPER_VCM_DC_SETTING_REGISTER                     0x82
#define E_PAPER_PROGRAM_MODE                                0xA0
#define E_PAPER_ACTIVE_PROGRAM                              0xA1
#define E_PAPER_READ_OTP_DATA                               0xA2

namespace EPaper {

    enum Rotation {
        E_PAPER_ROTATE_0,
        E_PAPER_ROTATE_90,
        E_PAPER_ROTATE_180,
        E_PAPER_ROTATE_270,
    };

    struct Config {
        /* GPIO Pins */
        int busy_pin;
        int reset_pin;
        uint8_t rst_active_level;
        /* Data/Control: HIGH -> write data, LOW -> write command */
        int dc_pin;
        uint8_t dc_level_data;
        uint8_t dc_level_cmd;

        /* SPI Pins */
        int cs_pin;
        int mosi_pin;
        int miso_pin;
        int sck_pin;

        int clk_freq_hz;

        int width;
        int height;
        bool color_inv;
    };

    class Display {

    public:


        /**
         * @brief Describes the state of the D/C line (Data/Control)
         */
        struct DCState {
            uint8_t dc_pin;
            uint8_t dc_level;
        };

        /**
         * @brief Create and init epaper display handler
         * @param conf configuration data
         */
        Display(const Config* conf); //iot_epaper_create
        virtual ~Display(); //iot_epaper_delete

        Rotation Rotate() { return m_rotate; };
        void Rotate(Rotation rotate) { m_rotate = rotate; };

        bool isInverted() { return m_color_invert; }
        /**
         * @brief Invert display colors. Attention! This affects only the pixels drawn to the framebuffer after this method is called. A already drawn pixel will not be inverted!
         */
        void Invert() { m_color_invert = !m_color_invert; }

        /**
         * @brief Get display width (rotation-aware)
         */
        int CanvasWidth();
        /**
         * @brief Get display height (rotation-aware)
         */
        int CanvasHeight();

        /**
         * @brief Show the current framebuffer
         */
        void ShowFramebuffer();
        /**
         * @brief Clear the framebuffer
         */
        void ClearFramebuffer();
        /**
         * @brief Draw a pixel on the framebuffer (rotation-aware)
         */
        void drawPixel(int x, int y);

    private:

        const Config* m_config;
        Display::DCState m_dc_state;

        Rotation m_rotate;
        int m_width;
        int m_height;

        spi_device_handle_t m_handle;
        spi_host_device_t m_host;

        uint8_t* m_framebuffer;
        size_t m_framebuffer_size;

        bool m_color_invert;

        /**
         * @brief Init GPIO Pins and SPI Bus, send init commands to the EPD
         */
        void init();

        /**
         * @brief Raw SPI data transfer (single byte only!)
         */
        void sendByte(const uint8_t payload);

        void sendCommand(const uint8_t command);
        void sendDataByte(const uint8_t data);
        void sendData(const uint8_t* data, int length);

        /**
         * @brief Wait while the display is busy
         */
        void waitWhileBusy();

        /**
         * @brief Draw a pixel on the framebuffer in absolute coordinates (independent of the rotation)
         */
        void drawPixelAbsolute(int x, int y);
    };
} // namespace EPaper
#endif // _EPAPER_DRIVER_H