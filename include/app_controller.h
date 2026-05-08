#pragma once

#include <Arduino.h>
#include "led_control.h"
#include "state_store.h"

namespace AppController {

void begin();
void loop();

DeviceState& state();
void setLed(uint8_t red, uint8_t green, uint8_t blue, bool persist = true);
void setLedConfig(const LedConfig& config, bool persist = true);
void setLedEffect(uint8_t effect, uint8_t speed, bool persist = true);
void setOledText(const String& text, bool persist = true);
void setOledTextStyle(uint8_t size, uint8_t align, uint8_t y, bool bold, bool persist = true);
void setOledScroll(bool enabled, uint8_t speed, bool persist = true);
void clearOled(bool persist = true);
void setOledInverted(bool enabled, bool persist = true);

}  // namespace AppController
