// Minimal Adafruit_GFX / Print stub - syntax checking only.
#ifndef HOST_GFX_H
#define HOST_GFX_H
#include <cstdint>
#include <cstdio>
class Print {
 public:
  void print(const char *s);
  void print(int v);
  void print(unsigned v);
  void println(const char *s);
  void println();
  template <typename... A> void printf(const char *f, A... a) { (void)f; }
};
class Adafruit_GFX : public Print {
 public:
  Adafruit_GFX(int16_t w, int16_t h);
  void setCursor(int16_t x, int16_t y);
  void setTextSize(uint8_t s);
  void setTextColor(uint16_t c);
  void drawLine(int16_t, int16_t, int16_t, int16_t, uint16_t);
  void drawRect(int16_t, int16_t, int16_t, int16_t, uint16_t);
  void fillRect(int16_t, int16_t, int16_t, int16_t, uint16_t);
  void drawBitmap(int16_t, int16_t, const uint8_t *, int16_t, int16_t, uint16_t);
};
#endif
