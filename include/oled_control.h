#pragma once

#include <Arduino.h>

namespace OledControl {

bool begin();
bool isReady();
void showBoot(const String& line1, const String& line2 = "");
void showText(const String& text, uint8_t size, uint8_t align, uint8_t y, bool bold);
void showScrollingText(const String& text, uint8_t size, uint8_t y, bool bold, int16_t x);
int16_t estimateTextWidth(const String& text, uint8_t size);
void clear();
void setInverted(bool enabled);
bool isInverted();
void selfTest();

}  // namespace OledControl
