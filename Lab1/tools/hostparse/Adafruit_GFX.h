#pragma once
#include "Arduino.h"
struct Adafruit_GFX : Printish {
  void setTextSize(uint8_t size);
  void setTextColor(uint16_t color);
  void setCursor(int16_t x, int16_t y);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
};
