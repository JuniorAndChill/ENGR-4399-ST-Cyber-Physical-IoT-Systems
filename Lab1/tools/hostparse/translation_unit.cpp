// Wraps the sketch in a translation unit the host compiler can parse.
#include "Arduino.h"
#include "../../sketch/sketch.ino"
int main() { setup(); loop(); return 0; }
