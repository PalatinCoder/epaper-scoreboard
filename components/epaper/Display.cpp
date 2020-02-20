#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <cstring>
#include "epaper.h"

using namespace EPaper;

static const char* TAG = "EPaper::Display";

static const uint8_t lut_vcom_dc[] = { 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00,
        0x05, 0x00, 0x32, 0x32, 0x00, 0x00, 0x02, 0x00, 0x0F, 0x0F, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00 };
//R21H
static const uint8_t lut_ww[] = { 0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x60, 0x32,
        0x32, 0x00, 0x00, 0x02, 0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//R22H    r
static const uint8_t lut_bw[] = { 0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x60, 0x32,
        0x32, 0x00, 0x00, 0x02, 0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//R24H    b
static const uint8_t lut_bb[] = { 0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x60, 0x32,
        0x32, 0x00, 0x00, 0x02, 0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//R23H    w
const unsigned char lut_wb[] = { 0xA0, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x60, 0x32,
        0x32, 0x00, 0x00, 0x02, 0x50, 0x0F, 0x0F, 0x00, 0x00, 0x05, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

Display::Display(const EPaper::Config* conf) {
    m_config = conf;
    m_handle = nullptr;
    m_host = HSPI_HOST;
    m_dc_state.dc_pin = m_config->dc_pin;
    m_height = m_config->height;
    m_color_invert = m_config->color_inv;

    init();
}

Display::~Display() {
    sendCommand(E_PAPER_POWER_OFF);

    heap_caps_free(m_framebuffer);

    ESP_LOGI(TAG, "Removing device");
    ESP_ERROR_CHECK(::spi_bus_remove_device(m_handle));

    ESP_LOGI(TAG, "Free bus");
    ESP_ERROR_CHECK(::spi_bus_free(m_host));
}

void Display::init() {

    // init GPIO Pins
    //iot_epaper_gpio_init()
    ESP_LOGD(TAG, "GPIO conf: rst=%d, dc=%d, busy=%d", this->m_config->reset_pin, this->m_config->dc_pin, this->m_config->busy_pin);

    gpio_pad_select_gpio                (this->m_config->reset_pin);
    gpio_set_direction      ((gpio_num_t)this->m_config->reset_pin, GPIO_MODE_OUTPUT);
    gpio_set_level          ((gpio_num_t)this->m_config->reset_pin, this->m_config->rst_active_level);

    gpio_pad_select_gpio                (this->m_config->dc_pin);
    gpio_set_direction      ((gpio_num_t)this->m_config->dc_pin, GPIO_MODE_OUTPUT);
    gpio_set_level          ((gpio_num_t)this->m_config->dc_pin, 1);
    ets_delay_us(10000);
    gpio_set_level          ((gpio_num_t)this->m_config->dc_pin, 0);

    gpio_pad_select_gpio                (this->m_config->busy_pin);
    gpio_set_direction      ((gpio_num_t)this->m_config->busy_pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode      ((gpio_num_t)this->m_config->busy_pin, GPIO_PULLUP_ONLY);

    // SPI init
    //iot_epaper_spi_init()
    ESP_LOGD(TAG, "SPI conf: mosi=%d, miso=%d, clk=%d, cs=%d", this->m_config->mosi_pin, this->m_config->miso_pin, this->m_config->sck_pin, this->m_config->cs_pin);

    spi_bus_config_t bus_config = {
        .mosi_io_num      = this->m_config->mosi_pin,
        .miso_io_num      = this->m_config->miso_pin,
        .sclk_io_num      = this->m_config->sck_pin,
        .quadwp_io_num    = 0,
        .quadhd_io_num    = 0,
        .max_transfer_sz  = 0, // use default -> 4kB
        .flags            = 0,
        .intr_flags       = 0
    };

    spi_device_interface_config_t dev_config = {
        .command_bits     = 0,
        .address_bits     = 0,
        .dummy_bits       = 0,
        .mode             = 0,
        .duty_cycle_pos   = 0,
        .cs_ena_pretrans  = static_cast<uint8_t>(EPAPER_CS_SETUP_NS / (EPAPER_1S_NS / (this->m_config->clk_freq_hz)) + 2),
        .cs_ena_posttrans = static_cast<uint8_t>(EPAPER_CS_HOLD_NS / (EPAPER_1S_NS / (this->m_config->clk_freq_hz)) + 2),
        .clock_speed_hz   = this->m_config->clk_freq_hz,
        .input_delay_ns   = 0,
        .spics_io_num     = this->m_config->cs_pin,
        .flags            = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),
        .queue_size       = EPAPER_QUEUE_SIZE_DEFAULT,
        // Pretransmit callback, sets the D/C line to the level in the transaction's user field
        .pre_cb           = [](spi_transaction_t* t){
            Display::DCState* dc = (Display::DCState*)t->user;
            gpio_set_level((gpio_num_t)dc->dc_pin, (int)dc->dc_level);
        },
        .post_cb          = nullptr
    };

    ESP_LOGI(TAG, "init spi bus");
    ESP_ERROR_CHECK(::spi_bus_initialize(m_host, &bus_config, 1));

    ESP_LOGI(TAG, "attach device to bus");
    ESP_ERROR_CHECK(::spi_bus_add_device(m_host, &dev_config, &m_handle));

    //iot_epaper_epd_init()
    /* EPD hardware init start */
    /* reset the display */
    gpio_set_level((gpio_num_t)m_config->reset_pin, (~(m_config->rst_active_level)) & 0x1);
    ets_delay_us(5);
    gpio_set_level((gpio_num_t)m_config->reset_pin, (m_config->rst_active_level) & 0x1);
    ets_delay_us(5);
    gpio_set_level((gpio_num_t)m_config->reset_pin, (~(m_config->rst_active_level)) & 0x1);
    this->waitWhileBusy();

    /* send init sequence */
    /* no clue what these commands all do, they are copied from the example code */
    sendCommand(E_PAPER_POWER_SETTING);
    sendDataByte(0x03);                  // VDS_EN, VDG_EN
    sendDataByte(0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    sendDataByte(0x2b);                  // VDH
    sendDataByte(0x2b);                  // VDL
    sendDataByte(0x09);                  // VDHR
    sendCommand(E_PAPER_BOOSTER_SOFT_START);
    sendDataByte(0x07);
    sendDataByte(0x07);
    sendDataByte(0x17);
    // Power optimization
    sendCommand(0xF8);
    sendDataByte(0x60);
    sendDataByte(0xA5);
    // Power optimization
    sendCommand(0xF8);
    sendDataByte(0x89);
    sendDataByte(0xA5);
    // Power optimization
    sendCommand(0xF8);
    sendDataByte(0x90);
    sendDataByte(0x00);
    // Power optimization
    sendCommand(0xF8);
    sendDataByte(0x93);
    sendDataByte(0x2A);
    // Power optimization
    sendCommand(0xF8);
    sendDataByte(0xA0);
    sendDataByte(0xA5);
    // Power optimization
    sendCommand(0xF8);
    sendDataByte(0xA1);
    sendDataByte(0x00);
    // Power optimization
    sendCommand(0xF8);
    sendDataByte(0x73);
    sendDataByte(0x41);
    sendCommand(E_PAPER_PARTIAL_DISPLAY_REFRESH);
    sendDataByte(0x00);
    sendCommand(E_PAPER_POWER_ON);
    waitWhileBusy();
    sendCommand(E_PAPER_PANEL_SETTING);
    sendDataByte(0xAF);            //KW-BF   KWR-AF    BWROTP 0f
    sendCommand(E_PAPER_PLL_CONTROL);
    sendDataByte(0x3A);          //3A 100HZ   29 150Hz 39 200HZ    31 171HZ
    sendCommand(E_PAPER_VCM_DC_SETTING_REGISTER);
    sendDataByte(0x12);

    /* set lut */
    sendCommand(E_PAPER_LUT_FOR_VCOM);
    sendData(lut_vcom_dc, sizeof(lut_vcom_dc));
    sendCommand(E_PAPER_LUT_WHITE_TO_WHITE);
    sendData(lut_ww, sizeof(lut_ww));
    sendCommand(E_PAPER_LUT_BLACK_TO_WHITE);
    sendData(lut_bw, sizeof(lut_bw));
    sendCommand(E_PAPER_LUT_WHITE_TO_BLACK);
    sendData(lut_wb, sizeof(lut_wb));
    sendCommand(E_PAPER_LUT_BLACK_TO_BLACK);
    sendData(lut_bb, sizeof(lut_bb));
    /* EPD hardware init end */

    //iot_epaper_paint_init()
    m_rotate = E_PAPER_ROTATE_0;
    m_height = m_config->height;
    /* 1 byte represents 8 pixels, so width should be a multiple of 8 */
    m_width = m_config->width % 8 ? m_config->width + 8 - (m_config->width % 8): m_config->width;

    m_framebuffer_size = m_width * m_height / 8; // pixel count divided by 8, as each bit represents a pixel
    m_framebuffer = (uint8_t*) heap_caps_calloc(1, m_framebuffer_size, MALLOC_CAP_8BIT); // alloc 1 chunk of memory, in the needed size. 
                                                                                         // calloc() clears the memory, so the framebuffer is filled with 0x00 (which is a black display)
    ESP_LOGI(TAG, "size of framebuffer: %d bytes", m_framebuffer_size);
}

void Display::waitWhileBusy() {
    while (gpio_get_level((gpio_num_t) m_config->busy_pin) == 0) {      //0: busy, 1: idle
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void Display::sendByte(const uint8_t payload) {
    ESP_LOGV(TAG, "send byte %#X", payload);
    // this EPD apparently supports only slow write mode, so we need to send byte by byte
    spi_transaction_t t = {
        .flags = 0,
        .cmd = 0,
        .addr = 0,
        .length = 8, // tx length, needed in bits
        .rxlength = 0,
        .user = &m_dc_state,
        .tx_buffer = &payload,
        .rx_buffer = nullptr
    };

    assert(::spi_device_transmit(m_handle, &t) == ESP_OK);
}

void Display::sendCommand(const uint8_t command) {
    m_dc_state.dc_level = m_config->dc_level_cmd;
    sendByte(command);
}

void Display::sendDataByte(const uint8_t data) {
    m_dc_state.dc_level = m_config->dc_level_data;
    sendByte(data);
}

void Display::sendData(const uint8_t* data, int length) {
    // this EPD apparently supports only slow write mode, so we need to send byte by byte
    for (size_t i = 0; i < length; i++)
    {
        sendDataByte(data[i]);
    }
}

void Display::ShowFramebuffer() {
    sendCommand(E_PAPER_DATA_START_TRANSMISSION_2);
    sendData(m_framebuffer, m_framebuffer_size);
    sendCommand(E_PAPER_DISPLAY_REFRESH);
    waitWhileBusy();
}

void Display::ClearFramebuffer() {
    // Fill the buffer white (0xFF) or black if inverted (0x00)
    char fill = m_color_invert ? 0x00 : 0xFF;
    memset(m_framebuffer, fill, m_framebuffer_size);
}

void Display::drawPixelAbsolute(int x, int y) {
    if (x < 0 || x > m_width || y < 0 || y >= m_height) return;

    //example enters critical here. should do it?
    if (m_color_invert) {
        m_framebuffer[(x + y * m_width) / 8] |=   0x80 >> (x % 8);
    } else {
        m_framebuffer[(x + y * m_width) / 8] &= ~(0x80 >> (x % 8));
    }
}

void Display::drawPixel(int x, int y) {
    switch (m_rotate)
    {
    case E_PAPER_ROTATE_0:
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
        drawPixelAbsolute(x, y);
        break;
    case E_PAPER_ROTATE_90:
        if (x < 0 || x >= m_height || y < 0 || y >= m_width) return;
        drawPixelAbsolute(m_width - y, x);
        break;
    case E_PAPER_ROTATE_180:
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
        drawPixelAbsolute(m_width - x, m_height - y);
        break;
    case E_PAPER_ROTATE_270:
        if (x < 0 || x >= m_height || y < 0 || y >= m_width) return;
        drawPixelAbsolute(y, m_height - x);
        break;
    default:
        break;
    }
}

int Display::CanvasWidth() {
    switch(m_rotate) {
        case E_PAPER_ROTATE_90:
        case E_PAPER_ROTATE_270:
            return m_height;
            break;
        case E_PAPER_ROTATE_0:
        case E_PAPER_ROTATE_180:
            return m_width;
            break;
        default:
            return -1;
    }
}

int Display::CanvasHeight() {
    switch(m_rotate) {
        case E_PAPER_ROTATE_90:
        case E_PAPER_ROTATE_270:
            return m_width;
            break;
        case E_PAPER_ROTATE_0:
        case E_PAPER_ROTATE_180:
            return m_height;
            break;
        default:
            return -1;
    }
}
