#include "app_controller.h"

#include "board_config.h"
#include "led_control.h"
#include "oled_control.h"
#include "state_store.h"
#include "web_server.h"
#include "wifi_ap.h"

namespace {

DeviceState currentState;
uint32_t lastStatusLogMs = 0;
uint32_t lastLedEffectMs = 0;
uint32_t lastOledScrollMs = 0;
int16_t oledScrollX = 0;
bool ledBlinkOn = true;
uint8_t ledPulseValue = 0;
int8_t ledPulseDirection = 1;
uint8_t ledRainbowPos = 0;

uint8_t clampByte(uint8_t value, uint8_t minimum, uint8_t maximum) {
  if (value < minimum) {
    return minimum;
  }
  if (value > maximum) {
    return maximum;
  }
  return value;
}

LedConfig ledConfigFromState() {
  LedConfig config;
  config.mode = BoardConfig::LedHardware::BuiltinRgb;
  config.brightness = clampByte(currentState.ledBrightness, 1, 255);
  config.rgbPin = BoardConfig::PIN_LED_RGB;
  return config;
}

uint8_t normalizedLedEffect(uint8_t effect) {
  return effect <= BoardConfig::LED_EFFECT_RAINBOW ? effect : BoardConfig::LED_EFFECT_SOLID;
}

uint8_t normalizedSpeed(uint8_t speed) {
  return clampByte(speed, 1, 10);
}

uint8_t normalizedTextSize(uint8_t size) {
  return clampByte(size, 1, BoardConfig::OLED_TEXT_SIZE_MAX);
}

uint8_t normalizedTextAlign(uint8_t align) {
  return align <= 2 ? align : 0;
}

uint8_t normalizedTextY(uint8_t y) {
  return clampByte(y, 0, BoardConfig::OLED_HEIGHT - 8);
}

uint16_t ledIntervalMs(uint8_t speed) {
  speed = normalizedSpeed(speed);
  return static_cast<uint16_t>(620 - (speed * 52));
}

uint16_t scrollIntervalMs(uint8_t speed) {
  speed = normalizedSpeed(speed);
  return static_cast<uint16_t>(155 - (speed * 12));
}

void persistIfNeeded(bool persist) {
  if (persist) {
    StateStore::save(currentState);
  }
}

void wheel(uint8_t pos, uint8_t& red, uint8_t& green, uint8_t& blue) {
  pos = 255 - pos;
  if (pos < 85) {
    red = 255 - pos * 3;
    green = 0;
    blue = pos * 3;
    return;
  }
  if (pos < 170) {
    pos -= 85;
    red = 0;
    green = pos * 3;
    blue = 255 - pos * 3;
    return;
  }
  pos -= 170;
  red = pos * 3;
  green = 255 - pos * 3;
  blue = 0;
}

void renderOledFromState() {
  if (currentState.oledScroll) {
    oledScrollX = 0;
    OledControl::showScrollingText(
        currentState.oledText,
        currentState.oledTextSize,
        currentState.oledTextY,
        currentState.oledTextBold,
        oledScrollX);
    return;
  }

  OledControl::showText(
      currentState.oledText,
      currentState.oledTextSize,
      currentState.oledTextAlign,
      currentState.oledTextY,
      currentState.oledTextBold);
}

void updateLedEffect(uint32_t now) {
  const uint8_t effect = normalizedLedEffect(currentState.ledEffect);
  if (effect == BoardConfig::LED_EFFECT_SOLID) {
    return;
  }

  const uint16_t interval = ledIntervalMs(currentState.ledEffectSpeed);
  if (now - lastLedEffectMs < interval) {
    return;
  }
  lastLedEffectMs = now;

  if (effect == BoardConfig::LED_EFFECT_BLINK) {
    ledBlinkOn = !ledBlinkOn;
    LedControl::setColor(
        ledBlinkOn ? currentState.ledRed : 0,
        ledBlinkOn ? currentState.ledGreen : 0,
        ledBlinkOn ? currentState.ledBlue : 0);
    return;
  }

  if (effect == BoardConfig::LED_EFFECT_PULSE) {
    const int next = static_cast<int>(ledPulseValue) + (ledPulseDirection * 24);
    if (next >= 255) {
      ledPulseValue = 255;
      ledPulseDirection = -1;
    } else if (next <= 0) {
      ledPulseValue = 0;
      ledPulseDirection = 1;
    } else {
      ledPulseValue = static_cast<uint8_t>(next);
    }
    LedControl::setColor(
        static_cast<uint8_t>((currentState.ledRed * ledPulseValue) / 255),
        static_cast<uint8_t>((currentState.ledGreen * ledPulseValue) / 255),
        static_cast<uint8_t>((currentState.ledBlue * ledPulseValue) / 255));
    return;
  }

  if (effect == BoardConfig::LED_EFFECT_RAINBOW) {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    wheel(ledRainbowPos, red, green, blue);
    ledRainbowPos += 8;
    LedControl::setColor(red, green, blue);
  }
}

void updateOledScroll(uint32_t now) {
  if (!currentState.oledScroll || currentState.oledText.length() == 0) {
    return;
  }

  if (now - lastOledScrollMs < scrollIntervalMs(currentState.oledScrollSpeed)) {
    return;
  }
  lastOledScrollMs = now;

  --oledScrollX;
  const int16_t textWidth = OledControl::estimateTextWidth(currentState.oledText, currentState.oledTextSize);
  if (oledScrollX < -textWidth) {
    oledScrollX = BoardConfig::OLED_WIDTH;
  }

  OledControl::showScrollingText(
      currentState.oledText,
      currentState.oledTextSize,
      currentState.oledTextY,
      currentState.oledTextBold,
      oledScrollX);
}

}  // namespace

namespace AppController {

void begin() {
  Serial.begin(BoardConfig::SERIAL_BAUD);
  delay(200);
  Serial.println();
  Serial.println("boot");

  if (!StateStore::begin()) {
    Serial.println("NVS init failed, running with defaults");
  }
  currentState = StateStore::load();
  currentState.ledMode = static_cast<uint8_t>(BoardConfig::LedHardware::BuiltinRgb);
  currentState.ledRgbPin = BoardConfig::PIN_LED_RGB;

  LedControl::begin();
  LedControl::configure(ledConfigFromState());
  LedControl::setColor(currentState.ledRed, currentState.ledGreen, currentState.ledBlue);
  LedControl::selfTest();

  if (OledControl::begin()) {
    OledControl::setInverted(currentState.oledInverted);
    renderOledFromState();
  } else {
    Serial.println("OLED init failed");
  }

  if (WifiAp::begin()) {
    Serial.print("AP: ");
    Serial.println(WifiAp::ssid());
    Serial.print("IP: ");
    Serial.println(WifiAp::ip());
  } else {
    Serial.println("AP start failed");
  }

  WebServerControl::begin();
  Serial.println("HTTP ready");
}

void loop() {
  WebServerControl::loop();

  const uint32_t now = millis();
  updateLedEffect(now);
  updateOledScroll(now);

  if (now - lastStatusLogMs > 30000) {
    lastStatusLogMs = now;
    Serial.print("clients=");
    Serial.print(WifiAp::connectedClients());
    Serial.print(" ip=");
    Serial.println(WifiAp::ip());
  }

  delay(1);
}

DeviceState& state() {
  return currentState;
}

void setLed(uint8_t red, uint8_t green, uint8_t blue, bool persist) {
  currentState.ledRed = red;
  currentState.ledGreen = green;
  currentState.ledBlue = blue;
  if (currentState.ledEffect == BoardConfig::LED_EFFECT_SOLID) {
    LedControl::setColor(red, green, blue);
  }
  persistIfNeeded(persist);
}

void setLedConfig(const LedConfig& config, bool persist) {
  currentState.ledMode = static_cast<uint8_t>(BoardConfig::LedHardware::BuiltinRgb);
  currentState.ledBrightness = clampByte(config.brightness, 1, 255);
  currentState.ledRgbPin = BoardConfig::PIN_LED_RGB;
  LedControl::configure(ledConfigFromState());
  persistIfNeeded(persist);
}

void setLedEffect(uint8_t effect, uint8_t speed, bool persist) {
  currentState.ledEffect = normalizedLedEffect(effect);
  currentState.ledEffectSpeed = normalizedSpeed(speed);
  lastLedEffectMs = 0;
  ledBlinkOn = true;
  ledPulseValue = 0;
  ledPulseDirection = 1;

  if (currentState.ledEffect == BoardConfig::LED_EFFECT_SOLID) {
    LedControl::setColor(currentState.ledRed, currentState.ledGreen, currentState.ledBlue);
  }

  persistIfNeeded(persist);
}

void setOledText(const String& text, bool persist) {
  currentState.oledText = text;
  renderOledFromState();
  persistIfNeeded(persist);
}

void setOledTextStyle(uint8_t size, uint8_t align, uint8_t y, bool bold, bool persist) {
  currentState.oledTextSize = normalizedTextSize(size);
  currentState.oledTextAlign = normalizedTextAlign(align);
  currentState.oledTextY = normalizedTextY(y);
  currentState.oledTextBold = bold;
  renderOledFromState();
  persistIfNeeded(persist);
}

void setOledScroll(bool enabled, uint8_t speed, bool persist) {
  currentState.oledScroll = enabled;
  currentState.oledScrollSpeed = normalizedSpeed(speed);
  lastOledScrollMs = 0;
  renderOledFromState();
  persistIfNeeded(persist);
}

void clearOled(bool persist) {
  currentState.oledText = "";
  OledControl::clear();
  persistIfNeeded(persist);
}

void setOledInverted(bool enabled, bool persist) {
  currentState.oledInverted = enabled;
  OledControl::setInverted(enabled);
  persistIfNeeded(persist);
}

}  // namespace AppController
