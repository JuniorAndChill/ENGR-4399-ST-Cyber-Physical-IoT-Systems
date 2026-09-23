/************************************************************
  songs_original.h - AUTO-GENERATED, do not edit by hand.
  Regenerate with:  python3 tools/compose.py

  Four original three-voice chiptunes written for this lab.
  Each song is three parallel scores - lead, harmony, bass - that
  carry identical total duration, which is what keeps the three
  buzzers locked together. tools/compose.py asserts that property
  bar by bar before emitting this file.

  Format: flat int array of {note, divider} pairs.
    divider  1 = whole, 2 = half, 4 = quarter, 8 = eighth,
            16 = sixteenth, negative = dotted (1.5x).
************************************************************/

#ifndef SONGS_ORIGINAL_H
#define SONGS_ORIGINAL_H

#include "notes.h"


// --------------------------------------------------------------------
//  Rival Encounter  -  Battle Theme
//  168 BPM, 16 bars, 22.9 s per pass
// --------------------------------------------------------------------
const int rivalLead[] = {
  A5,16, B5,16, C6,16, B5,16, A5,16, G5,16, A5,16, E5,16,
  F5,8, E5,8, D5,8, E5,8, A5,16, B5,16, C6,16, D6,16,
  E6,8, D6,8, C6,8, B5,8, A5,4, A5,16, B5,16, C6,16,
  B5,16, A5,16, G5,16, A5,16, E5,16, F5,8, E5,8, D5,8,
  E5,8, E6,8, D6,8, C6,8, B5,8, A5,4, E5,4, A5,4,
  C6,8, B5,8, A5,8, G5,8, E5,4, F5,4, A5,8, G5,8,
  F5,8, E5,8, D5,4, G5,4, B5,8, A5,8, G5,8, F5,8,
  E5,4, A5,2, REST,4, E6,4, C6,4, E6,8, D6,8, C6,8,
  B5,8, A5,4, D6,4, F6,8, E6,8, D6,8, C6,8, B5,4,
  E6,4, D6,8, C6,8, B5,8, A5,8, G5,4, A5,1, A5,16,
  G5,16, F5,16, E5,16, D5,16, C5,16, B4,16, A4,16, B4,8,
  C5,8, D5,8, E5,8, F5,16, E5,16, D5,16, C5,16, B4,16,
  A4,16, G4,16, A4,16, B4,8, D5,8, E5,4, A5,8, E5,8,
  A5,8, C6,8, B5,8, A5,8, E5,4, A5,4, C6,4, E6,2
};

const int rivalHarm[] = {
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  E4,8, GS4,8, B4,8, GS4,8, E4,8, GS4,8, B4,8, GS4,8,
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  F4,8, A4,8, C5,8, A4,8, F4,8, A4,8, C5,8, A4,8,
  G4,8, B4,8, D5,8, B4,8, G4,8, B4,8, D5,8, B4,8,
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  C5,8, E5,8, G5,8, E5,8, C5,8, E5,8, G5,8, E5,8,
  D5,8, F5,8, A5,8, F5,8, D5,8, F5,8, A5,8, F5,8,
  E4,8, GS4,8, B4,8, GS4,8, E4,8, GS4,8, B4,8, GS4,8,
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  F4,8, A4,8, C5,8, A4,8, F4,8, A4,8, C5,8, A4,8,
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  E4,8, GS4,8, B4,8, GS4,8, E4,8, GS4,8, B4,8, GS4,8
};

const int rivalBass[] = {
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  E3,8, REST,8, E3,8, REST,8, E3,8, REST,8, B3,8, REST,8,
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  F3,8, REST,8, F3,8, REST,8, F3,8, REST,8, C4,8, REST,8,
  G3,8, REST,8, G3,8, REST,8, G3,8, REST,8, D4,8, REST,8,
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  C4,8, REST,8, C4,8, REST,8, C4,8, REST,8, G4,8, REST,8,
  D4,8, REST,8, D4,8, REST,8, D4,8, REST,8, A4,8, REST,8,
  E3,8, REST,8, E3,8, REST,8, E3,8, REST,8, B3,8, REST,8,
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  F3,8, REST,8, F3,8, REST,8, F3,8, REST,8, C4,8, REST,8,
  A3,8, REST,8, A3,8, REST,8, A3,8, REST,8, E4,8, REST,8,
  E3,8, REST,8, E3,8, REST,8, E3,8, REST,8, B3,8, REST,8
};


// --------------------------------------------------------------------
//  Crimson Vanguard  -  Anime Opening
//  152 BPM, 16 bars, 25.3 s per pass
// --------------------------------------------------------------------
const int crimsonLead[] = {
  D5,8, E5,8, F5,4, E5,8, D5,8, A4,4, D5,8, F5,8,
  A5,4, G5,8, F5,8, E5,4, F5,8, G5,8, A5,4, AS5,8,
  A5,8, G5,4, F5,2, REST,4, A4,4, D5,8, E5,8, F5,4,
  A5,8, G5,8, F5,4, E5,8, F5,8, G5,4, F5,8, E5,8,
  D5,4, AS5,4, A5,8, G5,8, F5,4, E5,4, D5,2, REST,2,
  A5,4, AS5,8, A5,8, G5,4, F5,4, D6,4, C6,8, AS5,8,
  A5,4, G5,4, F5,8, G5,8, A5,8, AS5,8, C6,4, D6,4,
  A5,1, D6,8, C6,8, AS5,4, A5,8, G5,8, F5,4, E5,8,
  F5,8, G5,4, A5,8, AS5,8, C6,4, D6,4, A5,4, F5,4,
  D5,4, D5,2, REST,2
};

const int crimsonHarm[] = {
  REST,8, F5,8, REST,8, A5,8, REST,8, F5,8, REST,8, D5,8,
  REST,8, F5,8, REST,8, A5,8, REST,8, F5,8, REST,8, D5,8,
  REST,8, AS4,8, REST,8, D5,8, REST,8, AS4,8, REST,8, G4,8,
  REST,8, CS5,8, REST,8, E5,8, REST,8, CS5,8, REST,8, A4,8,
  REST,8, F5,8, REST,8, A5,8, REST,8, F5,8, REST,8, D5,8,
  REST,8, F5,8, REST,8, A5,8, REST,8, F5,8, REST,8, D5,8,
  REST,8, AS4,8, REST,8, D5,8, REST,8, AS4,8, REST,8, G4,8,
  REST,8, CS5,8, REST,8, E5,8, REST,8, CS5,8, REST,8, A4,8,
  REST,8, A4,8, REST,8, C5,8, REST,8, A4,8, REST,8, F4,8,
  REST,8, E5,8, REST,8, G5,8, REST,8, E5,8, REST,8, C5,8,
  REST,8, AS4,8, REST,8, D5,8, REST,8, AS4,8, REST,8, G4,8,
  REST,8, CS5,8, REST,8, E5,8, REST,8, CS5,8, REST,8, A4,8,
  REST,8, F5,8, REST,8, A5,8, REST,8, F5,8, REST,8, D5,8,
  REST,8, AS4,8, REST,8, D5,8, REST,8, AS4,8, REST,8, G4,8,
  REST,8, CS5,8, REST,8, E5,8, REST,8, CS5,8, REST,8, A4,8,
  REST,8, F5,8, REST,8, A5,8, REST,8, F5,8, REST,8, D5,8
};

const int crimsonBass[] = {
  D3,4, D3,8, REST,8, A3,4, D3,4, D3,4, D3,8, REST,8,
  A3,4, D3,4, G3,4, G3,8, REST,8, D4,4, G3,4, A3,4,
  A3,8, REST,8, E4,4, A3,4, D3,4, D3,8, REST,8, A3,4,
  D3,4, D3,4, D3,8, REST,8, A3,4, D3,4, G3,4, G3,8,
  REST,8, D4,4, G3,4, A3,4, A3,8, REST,8, E4,4, A3,4,
  F3,4, F3,8, REST,8, C4,4, F3,4, C4,4, C4,8, REST,8,
  G4,4, C4,4, G3,4, G3,8, REST,8, D4,4, G3,4, A3,4,
  A3,8, REST,8, E4,4, A3,4, D3,4, D3,8, REST,8, A3,4,
  D3,4, G3,4, G3,8, REST,8, D4,4, G3,4, A3,4, A3,8,
  REST,8, E4,4, A3,4, D3,4, D3,8, REST,8, A3,4, D3,4
};


// --------------------------------------------------------------------
//  Neon Alchemy  -  Slow Theme
//  120 BPM, 16 bars, 32.0 s per pass
// --------------------------------------------------------------------
const int neonLead[] = {
  B4,4, E5,4, G5,4, FS5,4, E5,2, B4,2, C5,4, E5,4,
  A5,4, G5,4, FS5,2, REST,2, B4,4, E5,4, G5,4, B5,4,
  A5,2, FS5,2, G5,4, FS5,8, E5,8, D5,4, B4,4, E5,1,
  G5,4, A5,4, B5,2, C6,4, B5,4, A5,2, G5,4, FS5,4,
  E5,4, D5,4, E5,2, REST,2, B5,4, A5,4, G5,4, FS5,4,
  E5,4, FS5,4, G5,2, A5,4, G5,4, FS5,4, E5,4, E5,1
};

const int neonHarm[] = {
  E4,8, B4,8, E5,8, B4,8, G4,8, B4,8, E5,8, B4,8,
  E4,8, B4,8, E5,8, B4,8, G4,8, B4,8, E5,8, B4,8,
  A4,8, E5,8, A5,8, E5,8, C5,8, E5,8, A5,8, E5,8,
  B3,8, FS4,8, B4,8, FS4,8, DS5,8, FS4,8, B4,8, FS4,8,
  E4,8, B4,8, E5,8, B4,8, G4,8, B4,8, E5,8, B4,8,
  D4,8, A4,8, D5,8, A4,8, FS4,8, A4,8, D5,8, A4,8,
  G4,8, D5,8, G5,8, D5,8, B4,8, D5,8, G5,8, D5,8,
  E4,8, B4,8, E5,8, B4,8, G4,8, B4,8, E5,8, B4,8,
  E4,8, B4,8, E5,8, B4,8, G4,8, B4,8, E5,8, B4,8,
  C5,8, G5,8, C6,8, G5,8, E5,8, G5,8, C6,8, G5,8,
  G4,8, D5,8, G5,8, D5,8, B4,8, D5,8, G5,8, D5,8,
  B3,8, FS4,8, B4,8, FS4,8, DS5,8, FS4,8, B4,8, FS4,8,
  E4,8, B4,8, E5,8, B4,8, G4,8, B4,8, E5,8, B4,8,
  C5,8, G5,8, C6,8, G5,8, E5,8, G5,8, C6,8, G5,8,
  A4,8, E5,8, A5,8, E5,8, C5,8, E5,8, A5,8, E5,8,
  E4,8, B4,8, E5,8, B4,8, G4,8, B4,8, E5,8, B4,8
};

const int neonBass[] = {
  E3,2, B3,4, E3,4, E3,2, B3,4, E3,4, A3,2, E4,4,
  A3,4, B3,2, FS4,4, B3,4, E3,2, B3,4, E3,4, D3,2,
  A3,4, D3,4, G3,2, D4,4, G3,4, E3,2, B3,4, E3,4,
  E3,2, B3,4, E3,4, C4,2, G4,4, C4,4, G3,2, D4,4,
  G3,4, B3,2, FS4,4, B3,4, E3,2, B3,4, E3,4, C4,2,
  G4,4, C4,4, A3,2, E4,4, A3,4, E3,2, B3,4, E3,4
};


// --------------------------------------------------------------------
//  Trap Arcade  -  Trap / 8-bit
//  142 BPM, 16 bars, 27.0 s per pass
// --------------------------------------------------------------------
const int trapLead[] = {
  FS5,4, FS5,8, E5,8, D5,4, B4,4, D5,8, E5,8, FS5,4,
  E5,4, D5,4, B4,4, D5,8, E5,8, FS5,2, E5,4, D5,4,
  B4,2, FS5,4, FS5,8, A5,8, FS5,4, E5,4, D5,8, E5,8,
  FS5,4, A5,4, FS5,4, B5,4, A5,8, FS5,8, E5,2, D5,4,
  B4,4, B4,2, FS5,16, FS5,16, FS5,8, E5,8, FS5,8, D5,4,
  REST,4, E5,16, E5,16, E5,8, D5,8, E5,8, B4,4, REST,4,
  FS5,8, A5,8, B5,4, A5,8, FS5,8, E5,4, D5,2, REST,2,
  FS5,4, FS5,8, E5,8, D5,4, B4,4, D5,8, E5,8, FS5,4,
  E5,4, D5,4, B4,4, D5,8, FS5,8, A5,4, FS5,4, B4,1
};

const int trapHarm[] = {
  FS5,8, FS5,16, FS5,16, D5,8, FS5,8, FS5,16, FS5,16, D5,8,
  FS5,8, B4,8, A5,8, A5,16, A5,16, FS5,8, A5,8, A5,16,
  A5,16, FS5,8, A5,8, D5,8, D5,8, D5,16, D5,16, B4,8,
  D5,8, D5,16, D5,16, B4,8, D5,8, G4,8, E5,8, E5,16,
  E5,16, CS5,8, E5,8, E5,16, E5,16, CS5,8, E5,8, A4,8,
  FS5,8, FS5,16, FS5,16, D5,8, FS5,8, FS5,16, FS5,16, D5,8,
  FS5,8, B4,8, A5,8, A5,16, A5,16, FS5,8, A5,8, A5,16,
  A5,16, FS5,8, A5,8, D5,8, D5,8, D5,16, D5,16, B4,8,
  D5,8, D5,16, D5,16, B4,8, D5,8, G4,8, E5,8, E5,16,
  E5,16, CS5,8, E5,8, E5,16, E5,16, CS5,8, E5,8, A4,8,
  FS5,8, FS5,16, FS5,16, D5,8, FS5,8, FS5,16, FS5,16, D5,8,
  FS5,8, B4,8, A5,8, A5,16, A5,16, FS5,8, A5,8, A5,16,
  A5,16, FS5,8, A5,8, D5,8, D5,8, D5,16, D5,16, B4,8,
  D5,8, D5,16, D5,16, B4,8, D5,8, G4,8, E5,8, E5,16,
  E5,16, CS5,8, E5,8, E5,16, E5,16, CS5,8, E5,8, A4,8,
  FS5,8, FS5,16, FS5,16, D5,8, FS5,8, FS5,16, FS5,16, D5,8,
  FS5,8, B4,8, A5,8, A5,16, A5,16, FS5,8, A5,8, A5,16,
  A5,16, FS5,8, A5,8, D5,8, D5,8, D5,16, D5,16, B4,8,
  D5,8, D5,16, D5,16, B4,8, D5,8, G4,8, FS5,8, FS5,16,
  FS5,16, D5,8, FS5,8, FS5,16, FS5,16, D5,8, FS5,8, B4,8
};

const int trapBass[] = {
  B2,4, REST,8, B2,8, B2,2, D3,4, REST,8, D3,8, D3,2,
  G2,4, REST,8, G2,8, G2,2, A2,4, REST,8, A2,8, A2,2,
  B2,4, REST,8, B2,8, B2,2, D3,4, REST,8, D3,8, D3,2,
  G2,4, REST,8, G2,8, G2,2, A2,4, REST,8, A2,8, A2,2,
  B2,4, REST,8, B2,8, B2,2, D3,4, REST,8, D3,8, D3,2,
  G2,4, REST,8, G2,8, G2,2, A2,4, REST,8, A2,8, A2,2,
  B2,4, REST,8, B2,8, B2,2, D3,4, REST,8, D3,8, D3,2,
  G2,4, REST,8, G2,8, G2,2, B2,4, REST,8, B2,8, B2,2
};

#endif  // SONGS_ORIGINAL_H
