// Minimal TwoWire stub - syntax checking only, see compile_check.sh.
#ifndef HOST_WIRE_H
#define HOST_WIRE_H
#include <cstdint>
class TwoWire {
 public:
  bool begin(int sda, int scl);
  void setClock(uint32_t hz);
};
extern TwoWire Wire;
#endif
