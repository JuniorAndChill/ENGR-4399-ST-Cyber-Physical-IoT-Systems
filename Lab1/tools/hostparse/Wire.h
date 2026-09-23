#pragma once
#include "Arduino.h"
struct TwoWire { void begin(int sda, int scl); };
extern TwoWire Wire;
