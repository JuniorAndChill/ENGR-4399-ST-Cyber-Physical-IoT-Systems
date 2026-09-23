// Stub Arduino core - declarations only, no behavior. See README.md.
#pragma once
#include <stdint.h>
#include <math.h>

#define HIGH 1
#define LOW  0
#define INPUT        0x01
#define OUTPUT       0x03
#define INPUT_PULLUP 0x05

// Core version, so the sketch's 2.x / 3.x PWM branches can both be parsed.
#ifndef STUB_CORE_MAJOR
#define STUB_CORE_MAJOR 3
#endif
#define ESP_ARDUINO_VERSION_VAL(ma, mi, pa) (((ma) << 16) | ((mi) << 8) | (pa))
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(STUB_CORE_MAJOR, 0, 0)

class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))

unsigned long millis();
void delay(unsigned long ms);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t level);
int  digitalRead(uint8_t pin);

// 3.x PWM API
void analogWrite(uint8_t pin, int value);
// 2.x LEDC API
void ledcSetup(uint8_t channel, double freq, uint8_t resolutionBits);
void ledcAttachPin(uint8_t pin, uint8_t channel);
void ledcWrite(uint8_t channel, uint32_t duty);

struct Printish {
  void print(const char *);            void println(const char *);
  void print(const __FlashStringHelper *); void println(const __FlashStringHelper *);
  void print(int);                     void println(int);
  void print(unsigned long);           void println(unsigned long);
  void print(uint32_t);                void println(uint32_t);
  void print(float, int);              void println(float, int);
};
struct SerialClass : Printish { void begin(unsigned long baud); };
extern SerialClass Serial;

struct EspClass { uint32_t getFreeHeap(); };
extern EspClass ESP;
