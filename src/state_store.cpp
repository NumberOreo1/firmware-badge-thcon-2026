#include "state_store.h"

#include <Preferences.h>

#include "board_config.h"

namespace {

Preferences preferences;
bool preferencesReady = false;
static constexpr uint8_t STATE_SCHEMA_VERSION = 4;

uint8_t clampByte(uint8_t value, uint8_t minimum, uint8_t maximum) {
  if (value < minimum) {
    return minimum;
  }
  if (value > maximum) {
    return maximum;
  }
  return value;
}

uint8_t loadUChar(const char* key, uint8_t fallback, uint8_t minimum, uint8_t maximum) {
  if (!preferencesReady) {
    return fallback;
  }
  return clampByte(preferences.getUChar(key, fallback), minimum, maximum);
}

}  // namespace

namespace StateStore {

bool begin() {
  preferencesReady = preferences.begin("badgectl", false);
  return preferencesReady;
}

DeviceState load() {
  DeviceState state;
  const uint8_t savedVersion = preferencesReady ? preferences.getUChar("cfg_ver", 0) : 0;
  state.ledRed = loadUChar("led_r", 0, 0, 255);
  state.ledGreen = loadUChar("led_g", 0, 0, 255);
  state.ledBlue = loadUChar("led_b", 0, 0, 255);
  state.ledEffect = loadUChar("led_eff", BoardConfig::LED_EFFECT_SOLID, 0, 3);
  state.ledEffectSpeed = loadUChar("led_eff_spd", BoardConfig::LED_EFFECT_SPEED, 1, 10);
  state.ledMode = loadUChar("led_mode", static_cast<uint8_t>(BoardConfig::LED_HARDWARE), 0, 3);
  state.ledBrightness = loadUChar("led_bright", BoardConfig::LED_BRIGHTNESS, 1, 255);
  state.ledRgbPin = loadUChar("led_rgb_pin", BoardConfig::PIN_LED_RGB, 0, 48);
  state.ledPwmRedPin = loadUChar("pwm_r_pin", BoardConfig::PIN_LED_RED, 0, 48);
  state.ledPwmGreenPin = loadUChar("pwm_g_pin", BoardConfig::PIN_LED_GREEN, 0, 48);
  state.ledPwmBluePin = loadUChar("pwm_b_pin", BoardConfig::PIN_LED_BLUE, 0, 48);
  state.ledPwmActiveLow = preferencesReady ? preferences.getBool("pwm_low", BoardConfig::PWM_LED_ACTIVE_LOW) : BoardConfig::PWM_LED_ACTIVE_LOW;
  state.oledText = preferencesReady ? preferences.getString("oled_text", BoardConfig::DEFAULT_OLED_TEXT) : BoardConfig::DEFAULT_OLED_TEXT;
  state.oledInverted = preferencesReady ? preferences.getBool("oled_inv", false) : false;
  state.oledTextSize = loadUChar("txt_size", BoardConfig::OLED_TEXT_SIZE, 1, BoardConfig::OLED_TEXT_SIZE_MAX);
  state.oledTextAlign = loadUChar("txt_align", BoardConfig::OLED_TEXT_ALIGN, 0, 2);
  state.oledTextY = loadUChar("txt_y", BoardConfig::OLED_TEXT_Y, 0, BoardConfig::OLED_HEIGHT - 8);
  state.oledScroll = preferencesReady ? preferences.getBool("txt_scroll", false) : false;
  state.oledScrollSpeed = loadUChar("txt_scr_spd", BoardConfig::OLED_SCROLL_SPEED, 1, 10);
  state.oledTextBold = preferencesReady ? preferences.getBool("txt_bold", BoardConfig::OLED_TEXT_BOLD) : BoardConfig::OLED_TEXT_BOLD;

  if (savedVersion < STATE_SCHEMA_VERSION) {
    state.ledMode = static_cast<uint8_t>(BoardConfig::LED_HARDWARE);
    state.ledEffect = BoardConfig::LED_EFFECT_SOLID;
    state.ledEffectSpeed = BoardConfig::LED_EFFECT_SPEED;
    state.ledBrightness = BoardConfig::LED_BRIGHTNESS;
    state.ledRgbPin = BoardConfig::PIN_LED_RGB;
    state.ledPwmRedPin = BoardConfig::PIN_LED_RED;
    state.ledPwmGreenPin = BoardConfig::PIN_LED_GREEN;
    state.ledPwmBluePin = BoardConfig::PIN_LED_BLUE;
    state.ledPwmActiveLow = BoardConfig::PWM_LED_ACTIVE_LOW;
  }

  return state;
}

void save(const DeviceState& state) {
  if (!preferencesReady) {
    return;
  }

  preferences.putUChar("led_r", state.ledRed);
  preferences.putUChar("led_g", state.ledGreen);
  preferences.putUChar("led_b", state.ledBlue);
  preferences.putUChar("led_eff", state.ledEffect);
  preferences.putUChar("led_eff_spd", state.ledEffectSpeed);
  preferences.putUChar("led_mode", state.ledMode);
  preferences.putUChar("led_bright", state.ledBrightness);
  preferences.putUChar("led_rgb_pin", state.ledRgbPin);
  preferences.putUChar("pwm_r_pin", state.ledPwmRedPin);
  preferences.putUChar("pwm_g_pin", state.ledPwmGreenPin);
  preferences.putUChar("pwm_b_pin", state.ledPwmBluePin);
  preferences.putBool("pwm_low", state.ledPwmActiveLow);
  preferences.putString("oled_text", state.oledText);
  preferences.putBool("oled_inv", state.oledInverted);
  preferences.putUChar("txt_size", state.oledTextSize);
  preferences.putUChar("txt_align", state.oledTextAlign);
  preferences.putUChar("txt_y", state.oledTextY);
  preferences.putBool("txt_scroll", state.oledScroll);
  preferences.putUChar("txt_scr_spd", state.oledScrollSpeed);
  preferences.putBool("txt_bold", state.oledTextBold);
  preferences.putUChar("cfg_ver", STATE_SCHEMA_VERSION);
}

void reset() {
  if (preferencesReady) {
    preferences.clear();
  }
}

}  // namespace StateStore
