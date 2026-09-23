#pragma once
#include "Arduino.h"
#define DHT11 11
#define DHT22 22
struct DHT {
  DHT(uint8_t pin, uint8_t type);
  void  begin();
  float readHumidity();
  float readTemperature();
};
