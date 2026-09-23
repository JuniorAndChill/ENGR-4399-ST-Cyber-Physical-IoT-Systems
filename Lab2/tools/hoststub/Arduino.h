/************************************************************
  tools/hoststub/Arduino.h

  Just enough of the Arduino and arduino-esp32 API for chiptune.h
  to compile on a desktop with g++. It exists so the sequencer can
  be built and exercised without hardware - see tools/host_sim.cpp.

  Nothing here pretends to emulate an ESP32. The LEDC functions
  record what they were asked to do; host_sim.cpp reads those
  records back out.
************************************************************/

#ifndef HOST_ARDUINO_H
#define HOST_ARDUINO_H

#include <cstdint>
#include <cstdio>
#include <cmath>

#define HIGH 1
#define LOW  0
#define PROGMEM
#define F(x) (x)

struct HostSerial {
  template <typename... A>
  void printf(const char *fmt, A... a) { std::printf(fmt, a...); }
  void println(const char *s) { std::puts(s); }
  void println() { std::puts(""); }
  void begin(unsigned long) {}
};
extern HostSerial Serial;

// --- LEDC shims. Defined in host_sim.cpp. ---
bool     ledcAttach(uint8_t pin, uint32_t freq, uint8_t bits);
uint32_t ledcWriteTone(uint8_t pin, uint32_t freq);
bool     ledcWrite(uint8_t pin, uint32_t duty);

uint32_t millis();
void     delay(uint32_t ms);
void     pinMode(uint8_t pin, uint8_t mode);
int      digitalRead(uint8_t pin);
void     digitalWrite(uint8_t pin, uint8_t val);

#define INPUT        0x01
#define OUTPUT       0x03
#define INPUT_PULLUP 0x05

struct HostESP { uint32_t getFreeHeap(); };
extern HostESP ESP;

#endif  // HOST_ARDUINO_H
