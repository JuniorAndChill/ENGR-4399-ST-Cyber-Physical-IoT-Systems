/************************************************************
  config.h - every tunable in one place
  Lab 2: ESP32 Three-Voice Chiptune Jukebox
************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

// ====================== PIN MAP ============================
// Buzzers. All three are passive piezo elements; each gets its
// own LEDC channel so the three can sound at the same time.
// GPIO 25/26 double as the DAC outputs, but LEDC drives them
// as ordinary PWM pins, which is all a passive buzzer needs.
#define PIN_LEAD    27
#define PIN_HARM    26
#define PIN_BASS    25

// Buttons - all active LOW on the internal pull-up, no resistors.
#define PIN_BTN_NEXT 32   // next track
#define PIN_BTN_PLAY 33   // play / pause
#define PIN_BTN_MIX   4   // cycle the voice mix (ALL / LEAD / LEAD+BASS)

// OLED SSD1306 128x64 over I2C
#define PIN_SDA      21
#define PIN_SCL      22
#define OLED_ADDR  0x3C
#define OLED_W      128
#define OLED_H       64

// ====================== AUDIO ==============================
// LEDC resolution. 10 bits gives 1024 duty steps, which is
// plenty for the coarse "volume" control below and keeps the
// usable carrier frequency well above the top note (E7).
#define LEDC_BITS    10
#define LEDC_MAX     ((1 << LEDC_BITS) - 1)

// Per-voice duty cycle, in percent of the PWM period.
//
// On a passive buzzer, duty does two things at once: it sets how
// much energy reaches the element (loudness) and it sets the
// harmonic content of the square wave (timbre). 50 % is a true
// square - loud and hollow. Narrower pulses are quieter and
// reedier, which is exactly the trick the NES pulse channels used
// to keep two square voices from masking each other.
#define DUTY_LEAD    50
#define DUTY_HARM    25
#define DUTY_BASS    40

// Articulation: each note is released slightly early so that two
// identical notes in a row are heard as two notes, not one long
// one. Expressed as a fraction of the note, with a floor and a
// ceiling in microseconds.
#define GATE_DIVISOR  6        // release after 5/6 of the note
#define GATE_MIN_US   8000UL   // never clip more than ... well, less than this
#define GATE_MAX_US  60000UL

// Small piezo elements are high-Q resonators and fall off steeply
// below roughly 200 Hz, so an octave-2 bass line can be inaudible
// on cheap parts. Set this to 12 to raise the bass voice one
// octave, or 0 to play the score as written.
#define BASS_OCTAVE_SHIFT 0

// How long each track should play for. The player repeats a track
// whole number of times, picking the count that lands closest to
// this target, so a 27 s song plays twice and a 32 s song plays
// twice. A track already longer than the target plays once.
#define TARGET_PLAY_MS 60000UL

// ====================== UI =================================
#define DEBOUNCE_MS       40
#define DISPLAY_MIN_MS    60    // fastest OLED refresh
#define DISPLAY_FORCE_MS 260    // refresh even without timing slack
// A full 128x64 frame is 1024 bytes; at 400 kHz that is ~23 ms of
// blocking I2C. The renderer therefore skips a refresh when a note
// onset is due within this window. See "Display scheduling" in the
// README.
#define DISPLAY_SLACK_US 26000UL
#define I2C_HZ         400000UL

#define SERIAL_BAUD    115200

#endif  // CONFIG_H
