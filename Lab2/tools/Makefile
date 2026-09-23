# Build and check the sequencer on a desktop, no board required.
#
#   make            build host_sim
#   make verify     build, then check the timeline against the score
#   make stress     the same check with a blocking display write, and
#                   again with a naive timer-driven redraw, for contrast
#   make songs      regenerate songs_original.h and sprite.h
#   make preview    render WAVs of every track into ../preview
#   make clean

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -I. -Ihoststub

host_sim: host_sim.cpp ../sketch/chiptune.h ../sketch/config.h \
          ../sketch/notes.h ../sketch/songs_original.h ../sketch/songs_classic.h
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: verify stress songs preview clean
verify: host_sim
	python3 verify.py

stress: host_sim
	python3 verify.py --display 23
	@echo
	python3 verify.py --late 23

songs:
	python3 compose.py
	python3 sprite_gen.py

preview:
	python3 render_wav.py

clean:
	rm -f host_sim events.csv sim.log

# Syntax-check the whole sketch - engine, playlist, display code - against
# minimal stubs for the Arduino and Adafruit APIs. Not a substitute for
# building in the Arduino IDE, but it catches type and syntax errors in
# seconds and without a toolchain download.
.PHONY: lint
lint:
	$(CXX) -fsyntax-only -std=c++17 -Wall -x c++ \
	    -Ihoststub -I../sketch ../sketch/sketch.ino
	@echo "sketch.ino: syntax ok"
