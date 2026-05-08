#pragma once

#include <Arduino.h>

#include "board_config.h"

struct LedConfig {
  BoardConfig::LedHardware mode = BoardConfig::LED_HARDWARE;
  uint8_t brightness = BoardConfig::LED_BRIGHTNESS;
  uint8_t rgbPin = BoardConfig::PIN_LED_RGB;
  uint8_t pwmRedPin = BoardConfig::PIN_LED_RED;
  uint8_t pwmGreenPin = BoardConfig::PIN_LED_GREEN;
  uint8_t pwmBluePin = BoardConfig::PIN_LED_BLUE;
  bool pwmActiveLow = BoardConfig::PWM_LED_ACTIVE_LOW;
};

namespace LedControl {

void begin();
void configure(const LedConfig& config);
LedConfig config();
const char* modeName(BoardConfig::LedHardware mode);
void setColor(uint8_t red, uint8_t green, uint8_t blue);
void off();
void selfTest();

}  // namespace LedControl
