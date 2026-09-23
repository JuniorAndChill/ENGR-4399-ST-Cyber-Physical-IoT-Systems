// Minimal Adafruit_SSD1306 stub - syntax checking only.
#ifndef HOST_SSD1306_H
#define HOST_SSD1306_H
#include "Adafruit_GFX.h"
#include "Wire.h"
#define SSD1306_WHITE        1
#define SSD1306_BLACK        0
#define SSD1306_SWITCHCAPVCC 2
class Adafruit_SSD1306 : public Adafruit_GFX {
 public:
  Adafruit_SSD1306(int16_t w, int16_t h, TwoWire *wire, int8_t rst);
  bool begin(uint8_t vcc, uint8_t addr);
  void clearDisplay();
  void display();
};
#endif
