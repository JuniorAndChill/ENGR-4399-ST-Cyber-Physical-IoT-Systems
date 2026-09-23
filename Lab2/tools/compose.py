#!/usr/bin/env python3
"""
compose.py - generates sketch/songs_original.h

The four original tracks that ship with Lab 2 are written here as structured
score data rather than typed straight into C. Two reasons:

  1. Every bar is checked. A three-voice arrangement only stays in sync if all
     three voices contain the same total duration; this script asserts that
     bar by bar and refuses to emit a score that drifts.
  2. The playback length at a given tempo is computed, not guessed, so the
     README's duration table is derived from the same source as the audio.

Durations use the same divider convention as the rest of the project:
    1 = whole, 2 = half, 4 = quarter, 8 = eighth, 16 = sixteenth,
    negative = dotted (1.5x).
A bar of 4/4 therefore sums to exactly 1.0 in whole-note units.

Usage:  python3 tools/compose.py            (writes sketch/songs_original.h)
        python3 tools/compose.py --check    (verify only, no write)
"""

import sys
import os
from fractions import Fraction

# --------------------------------------------------------------------------
# Score helpers
# --------------------------------------------------------------------------

def dur(divider):
    """Divider notation -> length in whole notes, as an exact Fraction."""
    if divider > 0:
        return Fraction(1, divider)
    return Fraction(3, 2) * Fraction(1, -divider)


def bar(*pairs):
    """One bar of 4/4. Raises if the contents do not sum to exactly 1.0."""
    total = sum(dur(d) for _, d in pairs)
    if total != 1:
        raise ValueError(f"bar sums to {total}, expected 1: {pairs}")
    return list(pairs)


def free(*pairs):
    """A run of notes that is not checked against a bar line (multi-bar holds)."""
    return list(pairs)


def voice(*bars):
    out = []
    for b in bars:
        out.extend(b)
    return out


def length_whole(v):
    return sum(dur(d) for _, d in v)


# Arpeggio / accompaniment pattern builders -------------------------------

def arp8(a, b, c, d):
    """Eight eighth-notes: a b c d a b c d. One bar."""
    return bar((a, 8), (b, 8), (c, 8), (d, 8), (a, 8), (b, 8), (c, 8), (d, 8))


def offbeat8(a, b, c, d):
    """Offbeat stabs: rest-a rest-b rest-c rest-d. One bar."""
    return bar((REST, 8), (a, 8), (REST, 8), (b, 8),
               (REST, 8), (c, 8), (REST, 8), (d, 8))


def broken8(r, f, o, t):
    """Broken arpeggio root-fifth-octave-fifth-third-fifth-octave-fifth."""
    return bar((r, 8), (f, 8), (o, 8), (f, 8), (t, 8), (f, 8), (o, 8), (f, 8))


def pulse_bass(root, fifth):
    """Staccato driving pulse: root rest root rest root rest fifth rest."""
    return bar((root, 8), (REST, 8), (root, 8), (REST, 8),
               (root, 8), (REST, 8), (fifth, 8), (REST, 8))


def walk_bass(root, fifth):
    """root(1/4) root(1/8) rest(1/8) fifth(1/4) root(1/4)."""
    return bar((root, 4), (root, 8), (REST, 8), (fifth, 4), (root, 4))


def hold_bass(root, fifth):
    """root(1/2) fifth(1/4) root(1/4)."""
    return bar((root, 2), (fifth, 4), (root, 4))


def trap_bass(root):
    """808 feel: root(1/4) rest(1/8) root(1/8) root(1/2)."""
    return bar((root, 4), (REST, 8), (root, 8), (root, 2))


def trap_hat(fifth, third, root):
    """Trap hi-hat feel with two sixteenth rolls per bar."""
    return bar((fifth, 8), (fifth, 16), (fifth, 16), (third, 8), (fifth, 8),
               (fifth, 16), (fifth, 16), (third, 8), (fifth, 8), (root, 8))


# --------------------------------------------------------------------------
# Note names  (mirrors sketch/notes.h - kept as strings so the emitted C
# uses the readable macros instead of raw integers)
# --------------------------------------------------------------------------
REST = "REST"
for _oct in range(2, 8):
    for _n in ["C", "CS", "D", "DS", "E", "F", "FS", "G", "GS", "A", "AS", "B"]:
        globals()[f"{_n}{_oct}"] = f"{_n}{_oct}"

FREQ = {"REST": 0}
_SEMI = {"C": -9, "CS": -8, "D": -7, "DS": -6, "E": -5, "F": -4,
         "FS": -3, "G": -2, "GS": -1, "A": 0, "AS": 1, "B": 2}
for _oct in range(2, 8):
    for _n, _s in _SEMI.items():
        n = _s + (_oct - 4) * 12
        FREQ[f"{_n}{_oct}"] = round(440.0 * (2.0 ** (n / 12.0)))


# ==========================================================================
#  SONG 1 - "Rival Encounter"          A minor, 168 BPM, 16 bars
#  A fast minor-key confrontation theme: sixteenth-note runs over a
#  driving staccato bass. Written to show off the lead voice.
# ==========================================================================
RIVAL_LEAD = voice(
    # -- intro riff --
    bar((A5,16),(B5,16),(C6,16),(B5,16),(A5,16),(G5,16),(A5,16),(E5,16),
        (F5,8),(E5,8),(D5,8),(E5,8)),
    bar((A5,16),(B5,16),(C6,16),(D6,16),(E6,8),(D6,8),(C6,8),(B5,8),(A5,4)),
    bar((A5,16),(B5,16),(C6,16),(B5,16),(A5,16),(G5,16),(A5,16),(E5,16),
        (F5,8),(E5,8),(D5,8),(E5,8)),
    bar((E6,8),(D6,8),(C6,8),(B5,8),(A5,4),(E5,4)),
    # -- main theme --
    bar((A5,4),(C6,8),(B5,8),(A5,8),(G5,8),(E5,4)),
    bar((F5,4),(A5,8),(G5,8),(F5,8),(E5,8),(D5,4)),
    bar((G5,4),(B5,8),(A5,8),(G5,8),(F5,8),(E5,4)),
    bar((A5,2),(REST,4),(E6,4)),
    bar((C6,4),(E6,8),(D6,8),(C6,8),(B5,8),(A5,4)),
    bar((D6,4),(F6,8),(E6,8),(D6,8),(C6,8),(B5,4)),
    bar((E6,4),(D6,8),(C6,8),(B5,8),(A5,8),(G5,4)),
    bar((A5,1)),
    # -- turnaround --
    bar((A5,16),(G5,16),(F5,16),(E5,16),(D5,16),(C5,16),(B4,16),(A4,16),
        (B4,8),(C5,8),(D5,8),(E5,8)),
    bar((F5,16),(E5,16),(D5,16),(C5,16),(B4,16),(A4,16),(G4,16),(A4,16),
        (B4,8),(D5,8),(E5,4)),
    bar((A5,8),(E5,8),(A5,8),(C6,8),(B5,8),(A5,8),(E5,4)),
    bar((A5,4),(C6,4),(E6,2)),
)

RIVAL_HARM = voice(
    arp8(A4, C5, E5, C5), arp8(A4, C5, E5, C5),
    arp8(A4, C5, E5, C5), arp8(E4, GS4, B4, GS4),
    arp8(A4, C5, E5, C5), arp8(F4, A4, C5, A4),
    arp8(G4, B4, D5, B4), arp8(A4, C5, E5, C5),
    arp8(C5, E5, G5, E5), arp8(D5, F5, A5, F5),
    arp8(E4, GS4, B4, GS4), arp8(A4, C5, E5, C5),
    arp8(A4, C5, E5, C5), arp8(F4, A4, C5, A4),
    arp8(A4, C5, E5, C5), arp8(E4, GS4, B4, GS4),
)

RIVAL_BASS = voice(
    pulse_bass(A3, E4), pulse_bass(A3, E4),
    pulse_bass(A3, E4), pulse_bass(E3, B3),
    pulse_bass(A3, E4), pulse_bass(F3, C4),
    pulse_bass(G3, D4), pulse_bass(A3, E4),
    pulse_bass(C4, G4), pulse_bass(D4, A4),
    pulse_bass(E3, B3), pulse_bass(A3, E4),
    pulse_bass(A3, E4), pulse_bass(F3, C4),
    pulse_bass(A3, E4), pulse_bass(E3, B3),
)


# ==========================================================================
#  SONG 2 - "Crimson Vanguard"         D minor, 152 BPM, 16 bars
#  Anime opening-theme shape: a long singable lead line, offbeat harmony
#  stabs on the and-of-each-beat, and a walking bass.
# ==========================================================================
CRIMSON_LEAD = voice(
    bar((D5,8),(E5,8),(F5,4),(E5,8),(D5,8),(A4,4)),
    bar((D5,8),(F5,8),(A5,4),(G5,8),(F5,8),(E5,4)),
    bar((F5,8),(G5,8),(A5,4),(AS5,8),(A5,8),(G5,4)),
    bar((F5,2),(REST,4),(A4,4)),
    bar((D5,8),(E5,8),(F5,4),(A5,8),(G5,8),(F5,4)),
    bar((E5,8),(F5,8),(G5,4),(F5,8),(E5,8),(D5,4)),
    bar((AS5,4),(A5,8),(G5,8),(F5,4),(E5,4)),
    bar((D5,2),(REST,2)),
    bar((A5,4),(AS5,8),(A5,8),(G5,4),(F5,4)),
    bar((D6,4),(C6,8),(AS5,8),(A5,4),(G5,4)),
    bar((F5,8),(G5,8),(A5,8),(AS5,8),(C6,4),(D6,4)),
    bar((A5,1)),
    bar((D6,8),(C6,8),(AS5,4),(A5,8),(G5,8),(F5,4)),
    bar((E5,8),(F5,8),(G5,4),(A5,8),(AS5,8),(C6,4)),
    bar((D6,4),(A5,4),(F5,4),(D5,4)),
    bar((D5,2),(REST,2)),
)

CRIMSON_HARM = voice(
    offbeat8(F5, A5, F5, D5), offbeat8(F5, A5, F5, D5),
    offbeat8(AS4, D5, AS4, G4), offbeat8(CS5, E5, CS5, A4),
    offbeat8(F5, A5, F5, D5), offbeat8(F5, A5, F5, D5),
    offbeat8(AS4, D5, AS4, G4), offbeat8(CS5, E5, CS5, A4),
    offbeat8(A4, C5, A4, F4), offbeat8(E5, G5, E5, C5),
    offbeat8(AS4, D5, AS4, G4), offbeat8(CS5, E5, CS5, A4),
    offbeat8(F5, A5, F5, D5), offbeat8(AS4, D5, AS4, G4),
    offbeat8(CS5, E5, CS5, A4), offbeat8(F5, A5, F5, D5),
)

CRIMSON_BASS = voice(
    walk_bass(D3, A3), walk_bass(D3, A3),
    walk_bass(G3, D4), walk_bass(A3, E4),
    walk_bass(D3, A3), walk_bass(D3, A3),
    walk_bass(G3, D4), walk_bass(A3, E4),
    walk_bass(F3, C4), walk_bass(C4, G4),
    walk_bass(G3, D4), walk_bass(A3, E4),
    walk_bass(D3, A3), walk_bass(G3, D4),
    walk_bass(A3, E4), walk_bass(D3, A3),
)


# ==========================================================================
#  SONG 3 - "Neon Alchemy"             E minor, 120 BPM, 16 bars
#  The slow one. Quarter-note lead over broken arpeggios - included so the
#  jukebox has a track where the three voices are individually audible.
# ==========================================================================
NEON_LEAD = voice(
    bar((B4,4),(E5,4),(G5,4),(FS5,4)),
    bar((E5,2),(B4,2)),
    bar((C5,4),(E5,4),(A5,4),(G5,4)),
    bar((FS5,2),(REST,2)),
    bar((B4,4),(E5,4),(G5,4),(B5,4)),
    bar((A5,2),(FS5,2)),
    bar((G5,4),(FS5,8),(E5,8),(D5,4),(B4,4)),
    bar((E5,1)),
    bar((G5,4),(A5,4),(B5,2)),
    bar((C6,4),(B5,4),(A5,2)),
    bar((G5,4),(FS5,4),(E5,4),(D5,4)),
    bar((E5,2),(REST,2)),
    bar((B5,4),(A5,4),(G5,4),(FS5,4)),
    bar((E5,4),(FS5,4),(G5,2)),
    bar((A5,4),(G5,4),(FS5,4),(E5,4)),
    bar((E5,1)),
)

NEON_HARM = voice(
    broken8(E4, B4, E5, G4), broken8(E4, B4, E5, G4),
    broken8(A4, E5, A5, C5), broken8(B3, FS4, B4, DS5),
    broken8(E4, B4, E5, G4), broken8(D4, A4, D5, FS4),
    broken8(G4, D5, G5, B4), broken8(E4, B4, E5, G4),
    broken8(E4, B4, E5, G4), broken8(C5, G5, C6, E5),
    broken8(G4, D5, G5, B4), broken8(B3, FS4, B4, DS5),
    broken8(E4, B4, E5, G4), broken8(C5, G5, C6, E5),
    broken8(A4, E5, A5, C5), broken8(E4, B4, E5, G4),
)

NEON_BASS = voice(
    hold_bass(E3, B3), hold_bass(E3, B3),
    hold_bass(A3, E4), hold_bass(B3, FS4),
    hold_bass(E3, B3), hold_bass(D3, A3),
    hold_bass(G3, D4), hold_bass(E3, B3),
    hold_bass(E3, B3), hold_bass(C4, G4),
    hold_bass(G3, D4), hold_bass(B3, FS4),
    hold_bass(E3, B3), hold_bass(C4, G4),
    hold_bass(A3, E4), hold_bass(E3, B3),
)


# ==========================================================================
#  SONG 4 - "Trap Arcade"              B minor, 142 BPM, 16 bars
#  Modern trap rhythm rendered in square waves: sparse pentatonic hook,
#  sixteenth-note hi-hat rolls in the harmony voice, and a sustained
#  sub-bass on the root. Bm - D - G - A throughout.
# ==========================================================================
TRAP_LEAD = voice(
    bar((FS5,4),(FS5,8),(E5,8),(D5,4),(B4,4)),
    bar((D5,8),(E5,8),(FS5,4),(E5,4),(D5,4)),
    bar((B4,4),(D5,8),(E5,8),(FS5,2)),
    bar((E5,4),(D5,4),(B4,2)),
    bar((FS5,4),(FS5,8),(A5,8),(FS5,4),(E5,4)),
    bar((D5,8),(E5,8),(FS5,4),(A5,4),(FS5,4)),
    bar((B5,4),(A5,8),(FS5,8),(E5,2)),
    bar((D5,4),(B4,4),(B4,2)),
    bar((FS5,16),(FS5,16),(FS5,8),(E5,8),(FS5,8),(D5,4),(REST,4)),
    bar((E5,16),(E5,16),(E5,8),(D5,8),(E5,8),(B4,4),(REST,4)),
    bar((FS5,8),(A5,8),(B5,4),(A5,8),(FS5,8),(E5,4)),
    bar((D5,2),(REST,2)),
    bar((FS5,4),(FS5,8),(E5,8),(D5,4),(B4,4)),
    bar((D5,8),(E5,8),(FS5,4),(E5,4),(D5,4)),
    bar((B4,4),(D5,8),(FS5,8),(A5,4),(FS5,4)),
    bar((B4,1)),
)

TRAP_HARM = voice(
    trap_hat(FS5, D5, B4), trap_hat(A5, FS5, D5),
    trap_hat(D5, B4, G4),  trap_hat(E5, CS5, A4),
    trap_hat(FS5, D5, B4), trap_hat(A5, FS5, D5),
    trap_hat(D5, B4, G4),  trap_hat(E5, CS5, A4),
    trap_hat(FS5, D5, B4), trap_hat(A5, FS5, D5),
    trap_hat(D5, B4, G4),  trap_hat(E5, CS5, A4),
    trap_hat(FS5, D5, B4), trap_hat(A5, FS5, D5),
    trap_hat(D5, B4, G4),  trap_hat(FS5, D5, B4),
)

TRAP_BASS = voice(
    trap_bass(B2), trap_bass(D3), trap_bass(G2), trap_bass(A2),
    trap_bass(B2), trap_bass(D3), trap_bass(G2), trap_bass(A2),
    trap_bass(B2), trap_bass(D3), trap_bass(G2), trap_bass(A2),
    trap_bass(B2), trap_bass(D3), trap_bass(G2), trap_bass(B2),
)


# ==========================================================================
SONGS = [
    dict(cid="rival",   title="Rival Encounter", style="Battle Theme",
         bpm=168, lead=RIVAL_LEAD,   harm=RIVAL_HARM,   bass=RIVAL_BASS),
    dict(cid="crimson", title="Crimson Vanguard", style="Anime Opening",
         bpm=152, lead=CRIMSON_LEAD, harm=CRIMSON_HARM, bass=CRIMSON_BASS),
    dict(cid="neon",    title="Neon Alchemy",    style="Slow Theme",
         bpm=120, lead=NEON_LEAD,    harm=NEON_HARM,    bass=NEON_BASS),
    dict(cid="trap",    title="Trap Arcade",     style="Trap / 8-bit",
         bpm=142, lead=TRAP_LEAD,    harm=TRAP_HARM,    bass=TRAP_BASS),
]

TARGET_MS = 60000


def analyse(song):
    """Verify the three voices agree, and compute playback length."""
    lens = {k: length_whole(song[k]) for k in ("lead", "harm", "bass")}
    if len(set(lens.values())) != 1:
        raise ValueError(f"{song['title']}: voice lengths disagree: {lens}")
    wholes = lens["lead"]
    beats = wholes * 4
    one_pass_s = float(beats) * 60.0 / song["bpm"]
    loops = max(1, round(TARGET_MS / 1000.0 / one_pass_s))
    return dict(bars=int(wholes), one_pass_s=one_pass_s,
                loops=loops, total_s=one_pass_s * loops,
                notes={k: len(song[k]) for k in ("lead", "harm", "bass")})


def emit_array(name, v):
    lines = [f"const int {name}[] = {{"]
    row = []
    for i, (n, d) in enumerate(v):
        row.append(f"{n},{d}")
        if len(row) == 8:
            lines.append("  " + " ".join(f"{t}," for t in row))
            row = []
    if row:
        lines.append("  " + " ".join(f"{t}," for t in row).rstrip(","))
    else:
        lines[-1] = lines[-1].rstrip(",")
    lines.append("};")
    return "\n".join(lines)


def main():
    check_only = "--check" in sys.argv
    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    out_path = os.path.join(here, "sketch", "songs_original.h")

    info = []
    for s in SONGS:
        a = analyse(s)
        info.append(a)
        print(f"{s['title']:<18} {s['bpm']:>4} BPM  {a['bars']:>3} bars  "
              f"pass {a['one_pass_s']:6.1f}s  x{a['loops']}  "
              f"= {a['total_s']:6.1f}s   "
              f"notes L/H/B {a['notes']['lead']}/{a['notes']['harm']}/{a['notes']['bass']}")

    if check_only:
        print("\nAll voices aligned.")
        return

    out = ['''/************************************************************
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
''']

    for s, a in zip(SONGS, info):
        out.append(f"""
// --------------------------------------------------------------------
//  {s['title']}  -  {s['style']}
//  {s['bpm']} BPM, {a['bars']} bars, {a['one_pass_s']:.1f} s per pass
// --------------------------------------------------------------------""")
        out.append(emit_array(f"{s['cid']}Lead", s["lead"]))
        out.append("")
        out.append(emit_array(f"{s['cid']}Harm", s["harm"]))
        out.append("")
        out.append(emit_array(f"{s['cid']}Bass", s["bass"]))
        out.append("")

    out.append("#endif  // SONGS_ORIGINAL_H")
    with open(out_path, "w") as f:
        f.write("\n".join(out) + "\n")
    print(f"\nwrote {out_path}")


if __name__ == "__main__":
    main()
