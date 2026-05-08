#pragma once

#include <Arduino.h>

#include "board_config.h"

struct DeviceState {
  uint8_t ledRed = 0;
  uint8_t ledGreen = 0;
  uint8_t ledBlue = 0;
  uint8_t ledEffect = BoardConfig::LED_EFFECT_SOLID;
  uint8_t ledEffectSpeed = BoardConfig::LED_EFFECT_SPEED;
  uint8_t ledMode = static_cast<uint8_t>(BoardConfig::LED_HARDWARE);
  uint8_t ledBrightness = BoardConfig::LED_BRIGHTNESS;
  uint8_t ledRgbPin = BoardConfig::PIN_LED_RGB;
  uint8_t ledPwmRedPin = BoardConfig::PIN_LED_RED;
  uint8_t ledPwmGreenPin = BoardConfig::PIN_LED_GREEN;
  uint8_t ledPwmBluePin = BoardConfig::PIN_LED_BLUE;
  bool ledPwmActiveLow = BoardConfig::PWM_LED_ACTIVE_LOW;
  String oledText;
  bool oledInverted = false;
  uint8_t oledTextSize = BoardConfig::OLED_TEXT_SIZE;
  uint8_t oledTextAlign = BoardConfig::OLED_TEXT_ALIGN;
  uint8_t oledTextY = BoardConfig::OLED_TEXT_Y;
  bool oledScroll = false;
  uint8_t oledScrollSpeed = BoardConfig::OLED_SCROLL_SPEED;
  bool oledTextBold = BoardConfig::OLED_TEXT_BOLD;
};

namespace StateStore {

bool begin();
DeviceState load();
void save(const DeviceState& state);
void reset();

}  // namespace StateStore
