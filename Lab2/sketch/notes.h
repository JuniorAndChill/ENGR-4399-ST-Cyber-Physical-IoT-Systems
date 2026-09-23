/************************************************************
  notes.h - Equal-tempered note frequency table (A4 = 440 Hz)
  Lab 2: ESP32 Three-Voice Chiptune Jukebox
  ------------------------------------------------------------
  Every macro is the fundamental frequency of that note in Hz,
  rounded to the nearest integer. The table follows

        f(n) = 440 * 2^(n/12)

  where n is the number of semitones away from A4. Rounding to
  whole hertz costs at most ~2 cents of pitch error in the top
  octave, which is far below what a piezo element resolves.

  Octave 2-3 exist for the bass voice. Note that a small passive
  piezo buzzer is a high-Q mechanical resonator: its output falls
  off steeply below roughly 200 Hz, so C2..B2 will be quiet or
  nearly inaudible on cheap elements. See BASS_OCTAVE_SHIFT in
  config.h for the compile-time fix.
************************************************************/

#ifndef NOTES_H
#define NOTES_H

#define REST 0

// ---- Octave 2 (bass; may be weak on small piezos) ----
#define C2    65
#define CS2   69
#define D2    73
#define DS2   78
#define E2    82
#define F2    87
#define FS2   93
#define G2    98
#define GS2  104
#define A2   110
#define AS2  117
#define B2   123

// ---- Octave 3 (bass / low harmony) ----
#define C3   131
#define CS3  139
#define D3   147
#define DS3  156
#define E3   165
#define F3   175
#define FS3  185
#define G3   196
#define GS3  208
#define A3   220
#define AS3  233
#define B3   247

// ---- Octave 4 ----
#define C4   262
#define CS4  277
#define D4   294
#define DS4  311
#define E4   330
#define F4   349
#define FS4  370
#define G4   392
#define GS4  415
#define A4   440
#define AS4  466
#define B4   494

// ---- Octave 5 ----
#define C5   523
#define CS5  554
#define D5   587
#define DS5  622
#define E5   659
#define F5   698
#define FS5  740
#define G5   784
#define GS5  831
#define A5   880
#define AS5  932
#define B5   988

// ---- Octave 6 ----
#define C6  1047
#define CS6 1109
#define D6  1175
#define DS6 1245
#define E6  1319
#define F6  1397
#define FS6 1480
#define G6  1568
#define GS6 1661
#define A6  1760
#define AS6 1865
#define B6  1976

// ---- Octave 7 (lead accents only) ----
#define C7  2093
#define D7  2349
#define E7  2637

// ---- Enharmonic aliases, so a score can be written in flats ----
#define DF5 CS5
#define EF5 DS5
#define GF5 FS5
#define AF5 GS5
#define BF5 AS5
#define BF4 AS4
#define EF4 DS4
#define BF3 AS3

#endif  // NOTES_H
