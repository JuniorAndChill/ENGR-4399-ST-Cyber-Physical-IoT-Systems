#pragma once
#include "Adafruit_GFX.h"
#include "Wire.h"
#define SSD1306_WHITE        1
#define SSD1306_SWITCHCAPVCC 0x02
struct Adafruit_SSD1306 : Adafruit_GFX {
  Adafruit_SSD1306(uint16_t w, uint16_t h, TwoWire *twi, int8_t rst);
  bool begin(uint8_t vcs, uint8_t addr);
  void clearDisplay();
  void display();
};
