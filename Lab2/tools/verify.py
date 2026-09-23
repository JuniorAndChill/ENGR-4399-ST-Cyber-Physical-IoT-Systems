#!/usr/bin/env python3
"""
verify.py - check the firmware's note timeline against the score.

Runs the compiled host simulator (tools/host_sim), which exercises the
real chiptune.h sequencer, and compares every note onset it produced
against the onset computed independently from the score data in
score.py. Two implementations of the same arithmetic, written from
opposite ends, have to agree.

Per track it reports:
  * notes expected, and how many the sequencer actually sounded
  * dropped / extra notes
  * the largest onset disagreement, in milliseconds
  * drift - whether that disagreement grows from the start of the track
    to the end. Drift is the failure the absolute-scheduling design in
    chiptune.h exists to prevent, so this is the number that matters.

Three runs are worth doing:

    python3 verify.py                 nothing else competing for the CPU
    python3 verify.py --display 23    with the real display scheduler,
                                      each frame write blocking 23 ms
    python3 verify.py --late 23       with a naive timer-driven redraw
                                      that stalls regardless of the music

The third is expected to drop notes. That contrast is the argument for
the slack check in serviceDisplay().
"""

import argparse
import os
import subprocess
import sys
from collections import defaultdict

import score

HERE = os.path.dirname(os.path.abspath(__file__))
SIM = os.path.join(HERE, "host_sim")


def run_sim(late=0, frame=0):
    if not os.path.exists(SIM):
        print("host_sim not built - run `make` in tools/ first", file=sys.stderr)
        sys.exit(1)
    cmd = [SIM]
    if late:
        cmd += ["--late", str(late)]
    if frame:
        cmd += ["--display", str(frame)]
    out = subprocess.run(cmd, capture_output=True, text=True, check=True)
    got = defaultdict(list)
    for line in out.stdout.splitlines()[1:]:
        s, v, t, f, _d = line.split(",")
        got[int(s)].append((int(v), int(t), int(f)))
    return got, out.stderr


def match(expected, played, tol):
    """
    Greedy in-order match of expected notes to played notes.

    Index-for-index comparison is useless once a single note is dropped -
    everything after it looks wrong. Walking both lists lets a drop be
    reported as one drop.

    Returns (errors, dropped, extra).
    """
    errs, dropped, extra = [], 0, 0
    j = 0
    for onset_ms, freq in expected:
        while j < len(played) and played[j][0] < onset_ms - tol:
            j += 1
            extra += 1
        if (j < len(played) and played[j][0] <= onset_ms + tol
                and played[j][1] == freq):
            errs.append(played[j][0] - onset_ms)
            j += 1
        else:
            dropped += 1
    extra += len(played) - j
    return errs, dropped, extra


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--late", type=int, default=0,
                    help="ms of blind stall every 10 steps (naive redraw)")
    ap.add_argument("--display", type=int, default=0,
                    help="ms a frame write blocks, using the real scheduler")
    ap.add_argument("--tolerance", type=int, default=3,
                    help="allowed onset error in ms (default 3)")
    args = ap.parse_args()

    playlist, notes, cfg = score.default_playlist()
    got, log = run_sim(args.late, args.display)

    stall = max(args.late, args.display)
    tol = args.tolerance + stall
    strict = args.late == 0        # a blind stall is allowed to drop notes

    mode = "idle CPU"
    if args.display:
        mode = f"real display scheduler, {args.display} ms per frame"
    if args.late:
        mode = f"naive redraw, {args.late} ms stall every 10 steps"
    print(f"mode: {mode}")
    print(f"tolerance: {tol} ms\n")
    print(f"{'track':<20}{'notes':>7}{'drop':>6}{'extra':>7}"
          f"{'max err':>9}{'drift':>8}{'len':>9}  result")
    print("-" * 78)

    failures = 0
    for i, (title, bpm, voices) in enumerate(playlist):
        events, _pass_us, _n, total_us = score.timeline(voices, bpm, cfg, notes)

        exp_by_v, act_by_v = defaultdict(list), defaultdict(list)
        for v, onset, _r, f in events:
            exp_by_v[v].append((-(-onset // 1000), f))   # ceil to the ms tick
        for v, t, f in got.get(i, []):
            act_by_v[v].append((t, f))

        all_errs, dropped, extra, n_exp = [], 0, 0, 0
        for v in sorted(exp_by_v):
            e = exp_by_v[v]
            n_exp += len(e)
            errs, d, x = match(e, act_by_v.get(v, []), tol)
            all_errs.extend(errs)
            dropped += d
            extra += x

        max_err = max((abs(x) for x in all_errs), default=0)
        # Guard both ends: a track where every note was dropped leaves
        # all_errs empty, and an unguarded max() over it raises rather
        # than reporting the drop.
        head = all_errs[:max(1, len(all_errs) // 10)] or [0]
        tail = all_errs[-max(1, len(all_errs) // 10):] or [0]
        drift = max(abs(x) for x in tail) - max(abs(x) for x in head)

        ok = max_err <= tol and abs(drift) <= tol and (
            not strict or (dropped == 0 and extra == 0))
        if not ok:
            failures += 1
        print(f"{title:<20}{n_exp:>7}{dropped:>6}{extra:>7}"
              f"{max_err:>7} ms{drift:>+6} ms{total_us/1e6:>8.1f}s  "
              f"{'ok' if ok else 'FAIL'}")

    print()
    if args.display:
        for line in log.strip().splitlines():
            if "frames" in line:
                print("  " + line.split("simulated")[0].strip() + "  "
                      + line.split("  frames")[1].strip().join(["frames ", ""]))
    if failures:
        print(f"{failures} track(s) failed")
        return 1
    if strict:
        print("all tracks match the score exactly; "
              "onset error does not grow with time")
    else:
        print("timing holds; see the drop column for notes lost to stalls")
    return 0


if __name__ == "__main__":
    sys.exit(main())
