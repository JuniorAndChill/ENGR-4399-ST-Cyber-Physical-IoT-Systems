#!/usr/bin/env python3
"""
import_song.py - pull {note, divider} score arrays out of a sketch and
check them before they go anywhere near the hardware.

The jukebox uses the same flat array format as the single-buzzer original,
so importing a score is mostly a matter of moving it. The part worth
automating is the checking:

  * how many bars each voice actually contains,
  * whether the voices of a multi-voice arrangement agree in length
    (they must, or the buzzers will slide apart),
  * where the first bar line falls that does not add up,
  * how long the result plays at a given tempo.

Examples
--------
Inspect every array in a file:

    python3 tools/import_song.py original/chiptune_asrun.ino

Check a three-voice arrangement and report bar-by-bar alignment:

    python3 tools/import_song.py my679.txt --voices fettyLead,fettyArp,fettyBass --bpm 190

Emit a header ready to #include:

    python3 tools/import_song.py my679.txt --emit sketch/songs_user.h \\
        --voices fettyLead,fettyArp,fettyBass
"""

import argparse
import os
import re
import sys
from fractions import Fraction

ARRAY_RE = re.compile(
    r"(?:const\s+)?int\s+(\w+)\s*\[\s*\]\s*=\s*\{(.*?)\}\s*;", re.S)
TOKEN_RE = re.compile(r"[A-Za-z_]\w*|-?\d+")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    return text


def parse_arrays(text):
    """-> {name: [(note, divider), ...]}"""
    out = {}
    for m in ARRAY_RE.finditer(strip_comments(text)):
        name, body = m.group(1), m.group(2)
        toks = TOKEN_RE.findall(body)
        if len(toks) % 2:
            print(f"  ! {name}: {len(toks)} tokens - not an even "
                  f"note/duration count; last entry ignored", file=sys.stderr)
            toks = toks[:-1]
        pairs = []
        for i in range(0, len(toks), 2):
            try:
                div = int(toks[i + 1])
            except ValueError:
                print(f"  ! {name}: pair {i//2} has a non-numeric duration "
                      f"{toks[i+1]!r}", file=sys.stderr)
                continue
            pairs.append((toks[i], div))
        out[name] = pairs
    return out


def dur(divider):
    if divider == 0:
        return Fraction(0)
    if divider > 0:
        return Fraction(1, divider)
    return Fraction(3, 2) * Fraction(1, -divider)


def total(pairs):
    return sum(dur(d) for _, d in pairs)


def bar_report(pairs, name):
    """Walk the score and report every bar line that does not land cleanly."""
    pos = Fraction(0)
    bad = []
    for i, (n, d) in enumerate(pairs):
        start = pos
        pos += dur(d)
        # a note that straddles a bar line without ending on one
        if start < int(start) + 1 <= pos and pos != int(pos) and dur(d) < 1:
            bad.append((i, n, d, float(start), float(pos)))
    if bad:
        print(f"    note: {len(bad)} tie(s) cross a bar line in {name} (syncopation, "
              f"not an error); first at #{bad[0][0]} {bad[0][1]},{bad[0][2]} spans "
              f"bar {bad[0][3]:.3f}->{bad[0][4]:.3f}")
    return bad


def emit_array(name, pairs, per_row=8):
    lines = [f"const int {name}[] = {{"]
    row = []
    for n, d in pairs:
        row.append(f"{n},{d}")
        if len(row) == per_row:
            lines.append("  " + " ".join(f"{t}," for t in row))
            row = []
    if row:
        lines.append("  " + " ".join(f"{t}," for t in row))
    lines[-1] = lines[-1].rstrip(",")
    lines.append("};")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("source", help=".ino / .h / .txt file containing the arrays")
    ap.add_argument("--voices", help="comma-separated lead,harmony,bass array "
                                     "names to cross-check")
    ap.add_argument("--bpm", type=int, default=120, help="tempo for the "
                                                         "duration estimate")
    ap.add_argument("--emit", metavar="HEADER", help="write the arrays to a header")
    ap.add_argument("--guard", default=None, help="include guard for --emit")
    args = ap.parse_args()

    with open(args.source) as f:
        arrays = parse_arrays(f.read())

    if not arrays:
        print("no score arrays found", file=sys.stderr)
        return 1

    print(f"{os.path.basename(args.source)}: {len(arrays)} array(s), "
          f"durations quoted at {args.bpm} BPM\n")
    print(f"{'array':<22}{'notes':>7}{'bars':>9}{'seconds':>10}")
    print("-" * 48)
    for name, pairs in arrays.items():
        t = total(pairs)
        secs = float(t) * 4 * 60.0 / args.bpm
        bars = f"{float(t):.3f}".rstrip("0").rstrip(".")
        print(f"{name:<22}{len(pairs):>7}{bars:>9}{secs:>10.1f}")

    for name, pairs in arrays.items():
        bar_report(pairs, name)

    if args.voices:
        names = [v.strip() for v in args.voices.split(",")]
        missing = [n for n in names if n not in arrays]
        if missing:
            print(f"\nnot found in {args.source}: {', '.join(missing)}",
                  file=sys.stderr)
            return 1
        print("\nvoice alignment")
        print("-" * 48)
        lens = {n: total(arrays[n]) for n in names}
        lead = lens[names[0]]
        ok = True
        for n in names:
            delta = lens[n] - lead
            flag = "ok" if delta == 0 else f"OFF BY {float(delta):+.4f} bars"
            if delta != 0:
                ok = False
            print(f"  {n:<20}{float(lens[n]):>9.3f} bars   {flag}")
        if ok:
            secs = float(lead) * 4 * 60.0 / args.bpm
            loops = max(1, round(60.0 / secs)) if secs else 1
            print(f"\n  all voices agree: {float(lead):.0f} bars, "
                  f"{secs:.1f} s per pass, "
                  f"{loops} pass(es) = {secs*loops:.1f} s at {args.bpm} BPM")
        else:
            print("\n  voices disagree - the buzzers will slide apart. "
                  "Fix before importing.")
            return 1

    if args.emit:
        guard = args.guard or (os.path.basename(args.emit)
                               .upper().replace(".", "_").replace("-", "_"))
        body = [f"""/************************************************************
  {os.path.basename(args.emit)} - imported score data
  Generated by tools/import_song.py from {os.path.basename(args.source)}

  Flat {{note, divider}} arrays:
    divider  1 = whole, 2 = half, 4 = quarter, 8 = eighth,
            16 = sixteenth, negative = dotted (1.5x).
************************************************************/

#ifndef {guard}
#define {guard}

#include "notes.h"
"""]
        for name, pairs in arrays.items():
            t = total(pairs)
            body.append(f"\n// {name}: {len(pairs)} notes, "
                        f"{float(t):.0f} bars")
            body.append(emit_array(name, pairs))
        body.append(f"\n#endif  // {guard}")
        with open(args.emit, "w") as f:
            f.write("\n".join(body) + "\n")
        print(f"\nwrote {args.emit}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
