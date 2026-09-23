# Host-side syntax check

`check.sh` parses `../../sketch/sketch.ino` with the host C++ compiler against
the stub headers in this folder. It is **not** a build: the stubs only declare
the Arduino, Wire, Adafruit GFX/SSD1306 and DHT symbols the sketch uses, so the
check catches syntax, type and control-flow errors in the sketch itself and
nothing about the real libraries or the ESP32 core.

It compiles four configurations — arduino-esp32 2.x and 3.x, each with
`SIM_WOKWI` 0 and 1 — so the conditional PWM and sensor-type code is parsed on
every path rather than only the one the default `#define` selects.

    ./check.sh

Requires only `g++`.
