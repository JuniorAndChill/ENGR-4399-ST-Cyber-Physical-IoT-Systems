#!/usr/bin/env python3
"""
score.py - shared score reader for the Lab 2 tools.

Reads sketch/notes.h and the songs_*.h headers, and reproduces the
sequencer's note timeline using the same integer microsecond arithmetic
that chiptune.h uses on the ESP32. Integer, not floating point, and in
the same order: the whole point of verify.py is to catch a rounding
difference of a millisecond or two, so this has to round the same way
the firmware does.

Used by verify.py (checks the firmware against this) and render_wav.py
(turns this into audio).
"""

import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SKETCH = os.path.join(ROOT, "sketch")

ARRAY_RE = re.compile(
    r"(?:const\s+)?int\s+(\w+)\s*\[\s*\]\s*=\s*\{(.*?)\}\s*;", re.S)
TOKEN_RE = re.compile(r"[A-Za-z_]\w*|-?\d+")
DEFINE_RE = re.compile(r"^\s*#define\s+(\w+)\s+(-?\d+)\s*$", re.M)


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", " ", text)


def load_notes(path=None):
    """notes.h -> {macro: frequency}. Aliases are resolved."""
    path = path or os.path.join(SKETCH, "notes.h")
    text = strip_comments(open(path).read())
    table = {n: int(v) for n, v in DEFINE_RE.findall(text)}
    for m in re.finditer(r"^\s*#define\s+(\w+)\s+([A-Za-z]\w*)\s*$",
                         text, re.M):
        if m.group(2) in table:
            table[m.group(1)] = table[m.group(2)]
    table.setdefault("REST", 0)
    return table


def load_arrays(paths):
    out = {}
    for p in paths:
        for name, body in ARRAY_RE.findall(strip_comments(open(p).read())):
            toks = TOKEN_RE.findall(body)
            pairs = [(toks[i], int(toks[i + 1]))
                     for i in range(0, len(toks) - 1, 2)]
            out[name] = pairs
    return out


def load_config(path=None):
    path = path or os.path.join(SKETCH, "config.h")
    text = strip_comments(open(path).read())
    return {n: int(v) for n, v in DEFINE_RE.findall(text)}


# --- the same arithmetic chiptune.h performs ---------------------------

def whole_us(bpm):
    return 240000000 // bpm


def note_us(divider, wus):
    if divider == 0:
        return 0
    if divider > 0:
        return wus // divider
    return (wus * 3) // (2 * -divider)


def score_us(pairs, wus):
    return sum(note_us(d, wus) for _, d in pairs)


def passes_for(pass_us, target_ms):
    target = target_ms * 1000
    n = (2 * target + pass_us) // (2 * pass_us)
    return max(1, n)


def timeline(voices, bpm, cfg, notes):
    """
    Reproduce what the firmware will play.

    voices: list of up to 3 score arrays (lead first); None/[] = silent.
    Returns (events, pass_us, passes, total_us) where each event is
    (voice, onset_us, release_us, freq_hz).
    """
    wus = whole_us(bpm)
    gate_div = cfg.get("GATE_DIVISOR", 6)
    gate_min = cfg.get("GATE_MIN_US", 8000)
    gate_max = cfg.get("GATE_MAX_US", 60000)
    shift = cfg.get("BASS_OCTAVE_SHIFT", 0)
    target = cfg.get("TARGET_PLAY_MS", 60000)

    lead = voices[0] or []
    pass_us = score_us(lead, wus) or wus
    n_pass = passes_for(pass_us, target)
    total = pass_us * n_pass

    events = []
    for v, pairs in enumerate(voices):
        if not pairs:
            continue
        for p in range(n_pass):
            base = p * pass_us
            # The firmware services voices on whole-millisecond ticks and
            # realigns every voice the moment a pass boundary is crossed.
            # A note whose onset falls in the sliver between the last tick
            # of the pass and the boundary is therefore never sounded. That
            # sliver is real: integer microsecond truncation leaves a voice
            # made of eighth notes a few microseconds short of one made of
            # whole notes, so a short voice tries to wrap just before the
            # boundary. Modelling the tick is what makes this agree with
            # the hardware.
            boundary_tick = -(-(base + pass_us) // 1000)   # ceil to ms
            t = base
            idx = 0
            # voices shorter than the lead wrap; longer ones are cut off
            while t < base + pass_us:
                if idx >= len(pairs):
                    idx = 0
                name, div = pairs[idx]
                idx += 1
                d = note_us(div, wus) or 1000
                onset, t = t, t + d
                if -(-onset // 1000) >= boundary_tick:
                    break                       # realigned away
                gate = min(max(d // gate_div, gate_min), gate_max)
                if gate >= d:
                    gate = d // 2
                release = t - gate
                f = notes.get(name, 0)
                if f and v == 2 and shift:
                    k = shift
                    while k >= 12:
                        f *= 2
                        k -= 12
                    while k <= -12:
                        f //= 2
                        k += 12
                if f:
                    events.append((v, onset, release, f))
    events.sort(key=lambda e: (e[1], e[0]))
    return events, pass_us, n_pass, total


def default_playlist():
    """The playlist as sketch.ino declares it."""
    notes = load_notes()
    arrays = load_arrays([
        os.path.join(SKETCH, "songs_original.h"),
        os.path.join(SKETCH, "songs_classic.h"),
    ])
    spec = [
        ("Rival Encounter",  168, ["rivalLead", "rivalHarm", "rivalBass"]),
        ("Crimson Vanguard", 152, ["crimsonLead", "crimsonHarm", "crimsonBass"]),
        ("Neon Alchemy",     120, ["neonLead", "neonHarm", "neonBass"]),
        ("Trap Arcade",      142, ["trapLead", "trapHarm", "trapBass"]),
        ("Pokemon Battle",   180, ["pokemonBattle", None, None]),
        ("Attack on Titan",  150, ["attackOnTitan", None, None]),
        ("Fullmetal Alch.",  135, ["fullmetalAlchemist", None, None]),
        ("Super Mario",      200, ["superMario", None, None]),
    ]
    out = []
    for title, bpm, names in spec:
        out.append((title, bpm, [arrays.get(n) if n else None for n in names]))
    return out, notes, load_config()
