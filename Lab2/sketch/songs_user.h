/************************************************************
  songs_user.h - your own tracks
  Lab 2: ESP32 Three-Voice Chiptune Jukebox
  ------------------------------------------------------------
  Empty right now. This file holds any number of three-voice
  tracks; each one adds a row to USER_SONG_ENTRIES, which
  sketch.ino splices into its playlist.

  ADDING A TRACK

  If the score already exists in a sketch or a text file, don't
  paste anything - point the importer at it:

      python3 tools/import_song.py <file> \
          --voices <lead>,<harmony>,<bass> \
          --bpm 190 --title "My Track" --style "Trap" --install

  It reads arrays in any of the usual shapes - `int x[] = {...}`,
  `const int x[] = {...}`, `const int x[] PROGMEM = {...}` -
  checks that the three voices agree in length, refuses to install
  them if they do not, and rewrites this file. Installing a title
  that is already here replaces just that track and leaves the
  others alone, so you can re-import after an edit without losing
  anything.

  Drop --install to get the report without writing.

  Once installed, `make verify` and `make preview` cover your
  tracks alongside the built-in ones, so you can check the timing
  and hear them before flashing.

  ------------------------------------------------------------
  THE ONE RULE

  All three voices must contain the SAME TOTAL DURATION. The
  sequencer treats the lead as authoritative: a shorter harmony or
  bass loops back to its own beginning under the lead, and a longer
  one is cut off at the end of the pass. Both are useful on purpose
  - a one-bar bass ostinato under a sixteen-bar melody is a real
  arrangement technique - but an accidental mismatch of half a beat
  is just a track that sounds wrong.

  The importer catches that, and the sketch prints an [audit] line
  over Serial at boot for every voice whose length does not match
  its lead.

  A bar of 4/4 adds up to 1.0 when you sum 1/divider over the bar:
  four quarter notes, or eight eighths, or a half plus two quarters.

  ------------------------------------------------------------
  DURATION

  You do not need to write a minute of music. The player repeats a
  track the whole number of times that lands closest to
  TARGET_PLAY_MS (60 s by default, in config.h), so an 18-bar tune
  at 190 BPM runs about 23 s per pass and plays three times.

  Tempo is worth a moment's thought. The same score at 95 BPM plays
  at half speed and lands at one 45 s pass instead - a different
  feel, not just a different length. If eighth notes are carrying
  the groove, the faster reading is usually the one you want.

  ------------------------------------------------------------
  WRITING A BASS PART

  notes.h goes down to C2, so a bass line does not have to sit in
  octave 4 for want of a macro. Two things to watch:

    * Keep it below the other voices. A "bass" written in octave 4
      lands on top of the harmony and doubles it instead of holding
      the bottom - the commonest fault in a first arrangement.
    * A small passive piezo is a high-Q resonator and falls off
      steeply below roughly 200 Hz, so an octave-2 line that looks
      right on paper can be inaudible. Write it in octave 3, or
      leave it low and set BASS_OCTAVE_SHIFT to 12 in config.h.

  ------------------------------------------------------------
  BY HAND

  If you would rather not use the importer, the format this file
  needs is:

      #define HAVE_USER_SONGS 1

      const int user0Lead[] = { E5,4, D5,8, B4,8, A4,2 };
      const int user0Harm[] = { A4,8, CS5,8, E5,8, CS5,8,
                                A4,8, CS5,8, E5,8, CS5,8 };
      const int user0Bass[] = { A3,2, E3,2 };

      #define USER_SONG_ENTRIES \
        { "My Track", "Demo", 120, { SCORE(user0Lead), SCORE(user0Harm), SCORE(user0Bass) } },

  Add user1Lead / user1Harm / user1Bass and a second row for a
  second track, and so on. Every line of the macro except the last
  needs a trailing backslash, and the last row keeps its comma.
************************************************************/

#ifndef SONGS_USER_H
#define SONGS_USER_H

#include "notes.h"

#define HAVE_USER_SONGS 0

#endif  // SONGS_USER_H
