/************************************************************
  songs_user.h - your own three-voice track goes here
  Lab 2: ESP32 Three-Voice Chiptune Jukebox
  ------------------------------------------------------------
  THE FAST WAY

  If your score already exists in a sketch or a text file, do not
  paste anything. Point the importer at it and it writes this file
  for you:

      python3 tools/import_song.py <yourfile> \
          --voices <lead>,<harmony>,<bass> \
          --bpm 190 --title "My Track" --style "Trap" --install

  It reads arrays in any of the usual shapes - `int x[] = {...}`,
  `const int x[] = {...}`, `const int x[] PROGMEM = {...}` - checks
  that the three voices agree in length, refuses to install them if
  they do not, and sets HAVE_USER_SONG to 1. The track then appears
  at the end of the playlist, and `make preview` and `make verify`
  pick it up automatically along with everything else.

  Drop --install to just get the report without writing anything.

  ------------------------------------------------------------
  THE MANUAL WAY

  Paste your three score arrays over the placeholders below, keeping
  the names userLead / userHarm / userBass, fill in the title, style
  and BPM, and change HAVE_USER_SONG to 1.

  With HAVE_USER_SONG at 0, everything in here is compiled out and
  costs nothing.

  ------------------------------------------------------------
  THE ONE RULE

  All three voices must contain the SAME TOTAL DURATION. The
  sequencer treats the lead as authoritative: a shorter harmony or
  bass loops back to its own beginning under the lead, and a longer
  one is cut off at the end of the pass. Both are useful on purpose
  - a one-bar bass ostinato under a sixteen-bar melody is a real
  arrangement technique - but an accidental mismatch of half a beat
  is just a track that sounds wrong.

  Two things catch that for you: the importer above reports each
  voice's length in bars and names any that disagree, and the sketch
  prints an [audit] line over Serial at startup for every voice whose
  length does not match its lead.

  A bar of 4/4 adds up to 1.0 when you sum 1/divider over the bar:
  four quarter notes, or eight eighths, or a half plus two quarters.

  ------------------------------------------------------------
  DURATION

  You do not need to write a minute of music. The player repeats a
  track the whole number of times that lands closest to
  TARGET_PLAY_MS (60 s by default, in config.h), so an 18-bar tune
  at 190 BPM runs about 23 s per pass and plays three times.

  Tempo is worth a moment's thought. The same score at 95 BPM plays
  at half speed and lands at one 45 s pass instead - which is a
  different feel, not just a different length. If eighth notes are
  carrying the groove, the faster reading is usually the one you
  want.

  ------------------------------------------------------------
  BASS RANGE

  notes.h now goes down to C2, so a bass line no longer has to sit
  in octave 4 for want of a macro. Be aware that a small passive
  piezo is a high-Q resonator and falls off steeply below roughly
  200 Hz: an octave-2 line that looks right on paper can be almost
  inaudible on cheap parts. Either write the bass in octave 3, or
  leave it low and set BASS_OCTAVE_SHIFT to 12 in config.h.
************************************************************/

#ifndef SONGS_USER_H
#define SONGS_USER_H

#include "notes.h"

// Set to 1 once your arrays are in.
#define HAVE_USER_SONG 0

#define USER_SONG_TITLE "My Track"
#define USER_SONG_STYLE "User"
#define USER_SONG_BPM   190

#if HAVE_USER_SONG

// ---- Voice 1: lead ------------------------------------------------
// The melody. This voice decides how long one pass of the track is.
const int userLead[] = {
  // two bars of placeholder - replace the whole array
  E5,4, E5,8, D5,8, B4,4, A4,4,
  B4,4, D5,8, E5,8, E5,2
};

// ---- Voice 2: harmony --------------------------------------------
// Arpeggios, chord stabs, or a counter-melody. Keep it above the
// bass and mostly out of the lead's register, or the two square
// waves will mask each other.
const int userHarm[] = {
  A4,8, C5,8, E5,8, C5,8, A4,8, C5,8, E5,8, C5,8,
  G4,8, B4,8, D5,8, B4,8, G4,8, B4,8, D5,8, B4,8
};

// ---- Voice 3: bass -----------------------------------------------
// Roots, and not much else. Long notes read better than busy ones
// on a piezo.
const int userBass[] = {
  A3,2, E4,4, A3,4,
  G3,2, D4,4, G3,4
};

#endif  // HAVE_USER_SONG

#endif  // SONGS_USER_H
