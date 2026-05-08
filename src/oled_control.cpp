#include "oled_control.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "board_config.h"

namespace {

Adafruit_SSD1306 display(
    BoardConfig::OLED_WIDTH,
    BoardConfig::OLED_HEIGHT,
    &Wire,
    BoardConfig::PIN_OLED_RESET);

bool ready = false;
bool inverted = false;

uint8_t normalizeSize(uint8_t size) {
  if (size < 1) {
    return 1;
  }
  if (size > BoardConfig::OLED_TEXT_SIZE_MAX) {
    return BoardConfig::OLED_TEXT_SIZE_MAX;
  }
  return size;
}

uint8_t normalizeAlign(uint8_t align) {
  return align <= 2 ? align : 0;
}

uint8_t normalizeY(uint8_t y) {
  const uint8_t maxY = BoardConfig::OLED_HEIGHT - 8;
  return y <= maxY ? y : maxY;
}

String normalizeText(const String& input) {
  String output;
  output.reserve(min(input.length(), BoardConfig::API_MAX_TEXT_LENGTH));

  for (size_t index = 0; index < input.length() && output.length() < BoardConfig::API_MAX_TEXT_LENGTH; ++index) {
    const char value = input.charAt(index);
    if (value == '\r') {
      continue;
    }
    if (value == '\n' || value == '\t' || static_cast<unsigned char>(value) >= 32) {
      output += value;
    }
  }

  return output;
}

void drawLine(const String& line, int16_t y, uint8_t size, uint8_t align, bool bold) {
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t width = 0;
  uint16_t height = 0;

  display.setTextSize(size);
  display.getTextBounds(line, 0, y, &x1, &y1, &width, &height);

  int16_t x = 0;
  if (align == 1) {
    x = static_cast<int16_t>((BoardConfig::OLED_WIDTH - width) / 2);
  } else if (align == 2) {
    x = static_cast<int16_t>(BoardConfig::OLED_WIDTH - width);
  }
  if (x < 0) {
    x = 0;
  }

  display.setCursor(x, y);
  display.print(line);
  if (bold && x < BoardConfig::OLED_WIDTH - 1) {
    display.setCursor(x + 1, y);
    display.print(line);
  }
}

void renderSegment(const String& segment, uint8_t size, uint8_t align, bool bold, int16_t& y) {
  const int16_t lineHeight = static_cast<int16_t>(8 * size);
  const size_t calculatedMaxChars = BoardConfig::OLED_WIDTH / (6 * size);
  const size_t maxChars = calculatedMaxChars > 0 ? calculatedMaxChars : 1;
  size_t offset = 0;

  if (segment.length() == 0) {
    y += lineHeight;
    return;
  }

  while (offset < segment.length() && y < BoardConfig::OLED_HEIGHT) {
    size_t count = min(maxChars, segment.length() - offset);
    size_t nextOffset = offset + count;

    if (nextOffset < segment.length()) {
      int breakAt = -1;
      for (size_t index = offset; index < nextOffset; ++index) {
        if (segment.charAt(index) == ' ') {
          breakAt = static_cast<int>(index);
        }
      }
      if (breakAt > static_cast<int>(offset)) {
        count = static_cast<size_t>(breakAt) - offset;
        nextOffset = static_cast<size_t>(breakAt) + 1;
      }
    }

    String line = segment.substring(offset, offset + count);
    line.trim();
    drawLine(line, y, size, align, bold);
    y += lineHeight;

    while (nextOffset < segment.length() && segment.charAt(nextOffset) == ' ') {
      ++nextOffset;
    }
    offset = nextOffset;
  }
}

void renderLines(const String& text, uint8_t size, uint8_t align, uint8_t yOffset, bool bold) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(true);

  size = normalizeSize(size);
  align = normalizeAlign(align);
  yOffset = normalizeY(yOffset);

  int16_t y = yOffset;
  size_t start = 0;
  while (start <= text.length() && y < BoardConfig::OLED_HEIGHT) {
    const int newline = text.indexOf('\n', start);
    if (newline < 0) {
      renderSegment(text.substring(start), size, align, bold, y);
      break;
    }
    renderSegment(text.substring(start, newline), size, align, bold, y);
    start = static_cast<size_t>(newline) + 1;
  }

  display.display();
}

void renderScrollingLine(const String& text, uint8_t size, uint8_t yOffset, bool bold, int16_t x) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  size = normalizeSize(size);
  yOffset = normalizeY(yOffset);

  display.setTextSize(size);
  display.setCursor(x, yOffset);
  display.print(text);
  if (bold) {
    display.setCursor(x + 1, yOffset);
    display.print(text);
  }
  display.display();
}

}  // namespace

namespace OledControl {

bool begin() {
  Wire.begin(BoardConfig::PIN_I2C_SDA, BoardConfig::PIN_I2C_SCL);
  ready = display.begin(SSD1306_SWITCHCAPVCC, BoardConfig::OLED_I2C_ADDRESS);

  if (!ready) {
    return false;
  }

  display.clearDisplay();
  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(true);
  display.display();
  return true;
}

bool isReady() {
  return ready;
}

void showBoot(const String& line1, const String& line2) {
  if (!ready) {
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  if (line2.length() > 0) {
    display.println(line2);
  }
  display.display();
}

void showText(const String& text, uint8_t size, uint8_t align, uint8_t y, bool bold) {
  if (!ready) {
    return;
  }

  renderLines(normalizeText(text), size, align, y, bold);
}

void showScrollingText(const String& text, uint8_t size, uint8_t y, bool bold, int16_t x) {
  if (!ready) {
    return;
  }

  renderScrollingLine(normalizeText(text), size, y, bold, x);
}

int16_t estimateTextWidth(const String& text, uint8_t size) {
  size = normalizeSize(size);
  return static_cast<int16_t>(normalizeText(text).length() * 6 * size);
}

void clear() {
  if (!ready) {
    return;
  }

  display.clearDisplay();
  display.display();
}

void setInverted(bool enabled) {
  inverted = enabled;
  if (ready) {
    display.invertDisplay(enabled);
  }
}

bool isInverted() {
  return inverted;
}

void selfTest() {
  if (!ready) {
    return;
  }

  showBoot("OLED OK", "BadgeControl");
  delay(500);
}

}  // namespace OledControl
