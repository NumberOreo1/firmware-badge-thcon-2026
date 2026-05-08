#include "led_control.h"

#include <esp32-hal-rgb-led.h>

namespace {

LedConfig currentConfig;
uint8_t currentRed = 0;
uint8_t currentGreen = 0;
uint8_t currentBlue = 0;

uint8_t scaled(uint8_t value) {
  return static_cast<uint8_t>((static_cast<uint16_t>(value) * currentConfig.brightness) / 255);
}

void writeCurrentColor() {
  rgbLedWrite(currentConfig.rgbPin, scaled(currentRed), scaled(currentGreen), scaled(currentBlue));
}

}  // namespace

namespace LedControl {

void begin() {
  configure(currentConfig);
}

void configure(const LedConfig& config) {
  currentConfig.mode = BoardConfig::LedHardware::BuiltinRgb;
  currentConfig.brightness = config.brightness == 0 ? 1 : config.brightness;
  currentConfig.rgbPin = config.rgbPin;
  writeCurrentColor();
}

LedConfig config() {
  return currentConfig;
}

const char* modeName(BoardConfig::LedHardware) {
  return "builtin";
}

void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  currentRed = red;
  currentGreen = green;
  currentBlue = blue;
  writeCurrentColor();
}

void off() {
  setColor(0, 0, 0);
}

void selfTest() {
  const uint8_t savedRed = currentRed;
  const uint8_t savedGreen = currentGreen;
  const uint8_t savedBlue = currentBlue;

  setColor(255, 0, 0);
  delay(250);
  setColor(0, 255, 0);
  delay(250);
  setColor(0, 0, 255);
  delay(250);
  setColor(savedRed, savedGreen, savedBlue);
}

}  // namespace LedControl
