/************************************************************
  tools/host_sim.cpp - run the sequencer on a desktop

  Builds sketch/chiptune.h against the stub Arduino API in
  hoststub/ and steps the player through every track one
  millisecond at a time, recording each frequency change on each
  voice. The result is the exact note timeline the ESP32 would
  produce, as CSV, which tools/verify.py then checks against the
  score data independently and tools/render_wav.py turns into an
  audio preview.

  Why bother: the interesting failure in a multi-voice sequencer
  is drift of a few milliseconds per bar, which is inaudible for
  ten seconds and obvious after sixty. Stepping it on a host and
  diffing the onsets against the arithmetic catches that in a
  second, and it does not need a board.

  Build and run:
      cd tools && make            # or see the g++ line in the Makefile
      ./host_sim > events.csv

  Options:
      ./host_sim --song 3         only track 3 (0-based)
      ./host_sim --step 1         simulation step in ms (default 1)
      ./host_sim --display 23     model the sketch's real display
                                  scheduler, with a frame write that
                                  blocks for this many ms
      ./host_sim --late 23        stall for this many ms every 10th
                                  step regardless of what the music is
                                  doing - i.e. what a naive "redraw on
                                  a timer" would do. The contrast with
                                  --display is the point.
************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>

#include "hoststub/Arduino.h"

HostSerial Serial;

// ---- LEDC recorder -------------------------------------------------
struct PinState { uint32_t freq = 0; uint32_t duty = 0; bool attached = false; };
static PinState g_pins[64];

bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t) {
  g_pins[pin].attached = true;
  g_pins[pin].freq = freq;
  g_pins[pin].duty = 0;
  return true;
}
uint32_t ledcWriteTone(uint8_t pin, uint32_t freq) {
  g_pins[pin].freq = freq;
  return freq;
}
bool ledcWrite(uint8_t pin, uint32_t duty) {
  g_pins[pin].duty = duty;
  return true;
}
static uint32_t g_millis = 0;
uint32_t millis() { return g_millis; }

// ---- the code under test -------------------------------------------
#include "../sketch/config.h"
#include "../sketch/notes.h"
#include "../sketch/chiptune.h"
#include "../sketch/songs_original.h"
#include "../sketch/songs_classic.h"

// Mirrors the PLAYLIST in sketch.ino. Kept separate so the harness
// does not need the Adafruit libraries.
static const Song PLAYLIST[] = {
  { "Rival Encounter",  "Battle Theme",  168,
    { SCORE(rivalLead),   SCORE(rivalHarm),   SCORE(rivalBass)   } },
  { "Crimson Vanguard", "Anime Opening", 152,
    { SCORE(crimsonLead), SCORE(crimsonHarm), SCORE(crimsonBass) } },
  { "Neon Alchemy",     "Slow Theme",    120,
    { SCORE(neonLead),    SCORE(neonHarm),    SCORE(neonBass)    } },
  { "Trap Arcade",      "Trap / 8-bit",  142,
    { SCORE(trapLead),    SCORE(trapHarm),    SCORE(trapBass)    } },
  { "Pokemon Battle",   "Lead only",     180,
    { SCORE(pokemonBattle),      NO_SCORE, NO_SCORE } },
  { "Attack on Titan",  "Lead only",     150,
    { SCORE(attackOnTitan),      NO_SCORE, NO_SCORE } },
  { "Fullmetal Alch.",  "Lead only",     135,
    { SCORE(fullmetalAlchemist), NO_SCORE, NO_SCORE } },
  { "Super Mario",      "Lead only",     200,
    { SCORE(superMario),         NO_SCORE, NO_SCORE } },
};
static const int NUM_SONGS = sizeof(PLAYLIST) / sizeof(PLAYLIST[0]);

static const uint8_t PINS[NUM_VOICES] = { PIN_LEAD, PIN_HARM, PIN_BASS };
static const uint8_t DUTY[NUM_VOICES] = { DUTY_LEAD, DUTY_HARM, DUTY_BASS };

int main(int argc, char **argv) {
  int only = -1, step = 1, late = 0, frame = 0;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--song") && i + 1 < argc) only = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--step") && i + 1 < argc) step = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--late") && i + 1 < argc) late = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--display") && i + 1 < argc) frame = atoi(argv[++i]);
  }
  if (step < 1) step = 1;

  ChiptunePlayer player;
  player.begin(PINS, DUTY);

  printf("song,voice,onset_ms,freq,duty\n");

  for (int s = 0; s < NUM_SONGS; s++) {
    if (only >= 0 && s != only) continue;

    uint32_t t = 0;
    g_millis = t;
    player.start(&PLAYLIST[s], t);

    uint32_t last_freq[NUM_VOICES] = {0, 0, 0};
    for (int v = 0; v < NUM_VOICES; v++) last_freq[v] = g_pins[PINS[v]].duty
                                                        ? g_pins[PINS[v]].freq : 0;

    uint32_t guard = player.totalMs() + 5000;
    long iter = 0;
    uint32_t lastDisplay = 0;
    long frames = 0, skipped = 0;
    while (!player.finished() && t < guard) {
      t += step;
      // A naive timer-driven redraw: stall no matter what the music
      // is doing.
      if (late > 0 && (++iter % 10) == 0) t += late;
      g_millis = t;
      player.update(t);

      // The scheduler the sketch actually uses: only spend the frame
      // time when the next note onset is far enough away.
      if (frame > 0) {
        uint32_t since = t - lastDisplay;
        if (since >= DISPLAY_MIN_MS) {
          if (since >= DISPLAY_FORCE_MS ||
              player.slackUs(t) >= DISPLAY_SLACK_US) {
            lastDisplay = t;
            t += frame;             // display() blocks for this long
            g_millis = t;
            frames++;
            player.update(t);
          } else {
            skipped++;
          }
        }
      }

      for (int v = 0; v < NUM_VOICES; v++) {
        uint32_t f = g_pins[PINS[v]].duty ? g_pins[PINS[v]].freq : 0;
        if (f != last_freq[v]) {
          if (f != 0)
            printf("%d,%d,%u,%u,%u\n", s, v, t, f, g_pins[PINS[v]].duty);
          last_freq[v] = f;
        }
      }
    }
    fprintf(stderr, "song %d  %-18s  reported %u ms  x%u passes  "
                    "simulated %u ms",
            s, PLAYLIST[s].title, player.totalMs(), player.passes(), t);
    if (frame > 0)
      fprintf(stderr, "  frames %ld drawn / %ld deferred (%.0f%% of "
                      "opportunities taken)",
              frames, skipped,
              100.0 * frames / (double)(frames + skipped ? frames + skipped : 1));
    fprintf(stderr, "\n");
  }
  return 0;
}
