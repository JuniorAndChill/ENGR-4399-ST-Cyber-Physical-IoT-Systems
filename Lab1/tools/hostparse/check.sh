#!/bin/sh
# Parse Lab 1's firmware with the host compiler against the stub headers here.
# Not a build - see README.md for what this does and does not prove.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
STATUS=0
for major in 2 3; do
  for sim in 0 1; do
    if g++ -std=gnu++17 -fsyntax-only -Wall -Wextra -Wno-unused-parameter \
         -I"$HERE" -DSTUB_CORE_MAJOR="$major" -DSIM_WOKWI="$sim" \
         "$HERE/translation_unit.cpp"; then
      echo "ok    arduino-esp32 ${major}.x, SIM_WOKWI=${sim}"
    else
      echo "FAIL  arduino-esp32 ${major}.x, SIM_WOKWI=${sim}"
      STATUS=1
    fi
  done
done
exit $STATUS
