#!/usr/bin/env python3
"""
render_wav.py - listen to a track before you wire anything up.

Takes the same note timeline the ESP32 will play (score.py, which
mirrors chiptune.h's integer arithmetic) and renders it to a WAV as
three square-wave voices, each at its configured duty cycle. It is not
a piezo model - a real buzzer has a pronounced resonance and no bass -
but it is accurate about pitch, rhythm, articulation and the balance
between the three voices, which is what you want to check before
committing a score to flash.

Requires numpy.

Usage:
    python3 tools/render_wav.py                  # every track
    python3 tools/render_wav.py --track trap     # one, by name fragment
    python3 tools/render_wav.py --out ~/Desktop  # somewhere else
"""

import argparse
import os
import sys
import wave

try:
    import numpy as np
except ImportError:
    sys.exit("render_wav.py needs numpy:  pip install numpy")

import score

RATE = 22050
ATTACK_S = 0.0015        # short ramp, so notes do not click


def square(freq, n, duty_pct, rate=RATE):
    t = np.arange(n) / rate
    phase = (t * freq) % 1.0
    return np.where(phase < duty_pct / 100.0, 1.0, -1.0)


def render(events, total_us, cfg, gains):
    n_total = int(total_us / 1e6 * RATE) + RATE // 4
    buf = np.zeros(n_total, dtype=np.float32)

    for v, onset, release, freq in events:
        a = int(onset / 1e6 * RATE)
        b = int(release / 1e6 * RATE)
        n = b - a
        if n <= 2 or a >= n_total:
            continue
        n = min(n, n_total - a)
        duty = gains[v][1]
        wave_ = square(freq, n, duty) * gains[v][0]
        ramp = max(2, int(ATTACK_S * RATE))
        if n > 2 * ramp:
            env = np.ones(n, dtype=np.float32)
            env[:ramp] = np.linspace(0, 1, ramp)
            env[-ramp:] = np.linspace(1, 0, ramp)
            wave_ = wave_ * env
        buf[a:a + n] += wave_.astype(np.float32)

    peak = float(np.max(np.abs(buf))) or 1.0
    buf = buf / peak * 0.85
    return (buf * 32767).astype("<i2")


def write_wav(path, samples):
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(samples.tobytes())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--track", help="render only tracks whose title contains this")
    ap.add_argument("--out", default=None, help="output directory")
    args = ap.parse_args()

    playlist, notes, cfg = score.default_playlist()
    out_dir = args.out or os.path.join(score.ROOT, "preview")
    os.makedirs(out_dir, exist_ok=True)

    # (amplitude, duty %) per voice. The amplitudes are the duty cycles
    # normalised, which is roughly how a buzzer's loudness tracks duty.
    gains = [
        (1.00, cfg.get("DUTY_LEAD", 50)),
        (0.55, cfg.get("DUTY_HARM", 25)),
        (0.75, cfg.get("DUTY_BASS", 40)),
    ]

    made = 0
    for title, bpm, voices in playlist:
        if args.track and args.track.lower() not in title.lower():
            continue
        events, _p, n_pass, total_us = score.timeline(voices, bpm, cfg, notes)
        samples = render(events, total_us, cfg, gains)
        name = title.lower().replace(" ", "_").replace(".", "") + ".wav"
        path = os.path.join(out_dir, name)
        write_wav(path, samples)
        print(f"{title:<20}{total_us/1e6:>7.1f}s  x{n_pass}  "
              f"{len(events):>4} notes  -> {path}")
        made += 1

    if not made:
        print("no track matched", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
