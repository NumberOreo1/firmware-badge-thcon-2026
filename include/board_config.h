#pragma once

#include <Arduino.h>

namespace BoardConfig {

static constexpr uint32_t SERIAL_BAUD = 115200;

static constexpr const char* AP_SSID = "Badge3000";
static constexpr const char* AP_PASSWORD = "badgecontrol";
static constexpr uint8_t AP_CHANNEL = 6;
static constexpr uint8_t AP_MAX_CONNECTIONS = 4;

static constexpr int OLED_WIDTH = 128;
static constexpr int OLED_HEIGHT = 64;
static constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
static constexpr int PIN_I2C_SDA = 6;
static constexpr int PIN_I2C_SCL = 7;
static constexpr int PIN_OLED_RESET = -1;

static constexpr size_t API_MAX_TEXT_LENGTH = 180;
static constexpr const char* DEFAULT_OLED_TEXT = "Badge ready";

enum class LedHardware : uint8_t {
  None = 0,
  Ws2812 = 1,
  PwmRgb = 2,
  BuiltinRgb = 3,
};

static constexpr LedHardware LED_HARDWARE = LedHardware::BuiltinRgb;
static constexpr int PIN_LED_RGB = 11;
static constexpr int PIN_LED_WS2812 = PIN_LED_RGB;
static constexpr uint16_t LED_WS2812_COUNT = 1;
static constexpr uint8_t LED_BRIGHTNESS = 96;
static constexpr uint8_t LED_EFFECT_SOLID = 0;
static constexpr uint8_t LED_EFFECT_BLINK = 1;
static constexpr uint8_t LED_EFFECT_PULSE = 2;
static constexpr uint8_t LED_EFFECT_RAINBOW = 3;
static constexpr uint8_t LED_EFFECT_SPEED = 5;

static constexpr int PIN_LED_RED = 3;
static constexpr int PIN_LED_GREEN = 4;
static constexpr int PIN_LED_BLUE = 5;
static constexpr bool PWM_LED_ACTIVE_LOW = false;
static constexpr uint32_t PWM_LED_FREQ = 5000;
static constexpr uint8_t PWM_LED_RESOLUTION = 8;
static constexpr uint8_t PWM_CHANNEL_RED = 0;
static constexpr uint8_t PWM_CHANNEL_GREEN = 1;
static constexpr uint8_t PWM_CHANNEL_BLUE = 2;

static constexpr uint8_t OLED_TEXT_SIZE = 1;
static constexpr uint8_t OLED_TEXT_SIZE_MAX = 8;
static constexpr uint8_t OLED_TEXT_ALIGN = 0;
static constexpr uint8_t OLED_TEXT_Y = 0;
static constexpr uint8_t OLED_SCROLL_SPEED = 5;
static constexpr bool OLED_TEXT_BOLD = false;

}  // namespace BoardConfig
