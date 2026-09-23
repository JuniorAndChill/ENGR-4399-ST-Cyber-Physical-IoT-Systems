# Lab 2 — Exercises

**ENGR 4399 ST: Cyber-Physical & IoT Systems**

Work through these in order; each builds on the last. Every exercise names what to
change, what to observe, and what to hand in. Most can be done entirely in Wokwi
or with the desktop tools in `tools/` — the ones that need a physical board are
marked **[hardware]**.

Before you start, get a baseline:

```bash
cd tools
make verify      # should report ok on all 8 tracks, drift within 1 ms
```

---

## Part A — Read the system (no code changes)

### A1. Map the pins

Without opening `config.h`, read `diagram.json` and fill in the table: which GPIO
drives which buzzer, which drives the OLED's two I²C lines, and which three are
buttons. Then check yourself against `config.h`.

**Hand in:** the completed table, and one sentence on how you can tell from
`diagram.json` alone that the buttons need no external resistors.

### A2. Trace one note

Pick any note in `rivalLead` in `songs_original.h`. Following `chiptune.h`, work
out by hand:

- the length of a whole note in microseconds at 168 BPM,
- that note's duration in microseconds,
- its gate (release) time,
- its absolute onset from the start of the track.

Then check your answer:

```bash
./host_sim --song 0 | head -20
```

**Hand in:** your arithmetic and the matching line from the simulator.

### A3. Why the duty cycle differs per voice

`config.h` sets the lead to 50 % duty, harmony to 25 %, bass to 40 %. Explain in
your own words what those numbers change about the sound — both in loudness and
in harmonic content. A Fourier series for a rectangular pulse train is the
starting point.

**Hand in:** a short paragraph, and a sketch of a 50 % and a 25 % pulse train over
one period.

---

## Part B — Modify the firmware

### B1. Retarget the play length

Change `TARGET_PLAY_MS` in `config.h` from 60 s to 30 s. Predict, before running
anything, how many times each of the eight tracks will repeat.

```bash
make verify
```

**Hand in:** your predictions next to the measured totals, and an explanation of
any you got wrong. (Hint: look at how `passes` rounds, and at which track already
exceeds the target.)

### B2. Add a fourth mix mode

`MixMode` in `chiptune.h` currently offers `ALL`, `LEAD`, `LEAD+BASS`. Add
`HARM+BASS` — accompaniment only, no melody — and make the `MIX` button cycle
through all four.

You will need to touch `MixMode`, `MIX_NAMES`, and `ChiptunePlayer::audible()`.

**Hand in:** your diff, and an explanation of why muting a voice does not desync
it. What in `service()` guarantees that un-muting lands in the right place?

### B3. Break the scheduler on purpose

In `ChiptunePlayer::service()`, replace the absolute schedule with a relative one:
instead of `st.nextUs = onset + d`, advance from the *current* time —
`st.nextUs = el + d`. This is the countdown design the README argues against.

```bash
make verify
```

Most tracks will now fail outright — once every onset shifts, essentially no note
matches its expected time any more. **But three tracks still pass.** Find out
which, and work out what those three have in common.

**Hand in:**

1. Which tracks still pass, and the property of their tempo that protects them.
   (Compute `240000000 / bpm` for each and look at the remainder.)
2. For a failing track, the per-voice error at the end of one pass, and the spread
   between the three voices — that spread is what you would hear. The README has
   the reference figures; yours should be close.
3. One sentence on why a countdown sequencer can pass every test a developer
   writes and still fall apart on someone else's tempo.

Then revert the change.

### B4. Make the display greedy

Comment out the slack test in `serviceDisplay()` so the sketch redraws purely on a
timer.

```bash
make stress
```

**Hand in:** the dropped-note counts, and an explanation of why exactly those
tracks lose notes and the others do not. Compute the shortest note duration in
each track at its tempo and compare it to the 23 ms frame cost.

### B5. Animate on the sixteenth **[harder]**

The sprite advances one frame per beat. Change it to advance on eighth notes, then
on sixteenths. At what subdivision does the animation stop reading as a dance, and
why? Relate your answer to the display's actual frame rate — `make stress` prints
frames drawn per track.

**Hand in:** the frame-rate figures and your conclusion about the useful upper
limit.

---

## Part C — Write music

### C1. Add a two-voice track

Write an eight-bar melody with a bass line into `songs_user.h`. Leave `userHarm`
empty (`NO_SCORE` in the playlist entry) so only two buzzers sound.

```bash
python3 tools/import_song.py sketch/songs_user.h \
    --voices userLead,userBass --bpm 120
python3 tools/render_wav.py --track "My Track"
```

**Hand in:** your score, the alignment report, and the rendered WAV.

### C2. Deliberately misalign it

Delete one eighth note from `userBass` and re-run the import check. Then build and
listen (or render) anyway.

**Hand in:** the tool's report, the `[audit]` line the sketch prints at boot, and a
description of what the misalignment sounds like over eight bars versus over
sixty seconds. This is the failure the "one rule" in `songs_user.h` exists to
prevent.

### C3. Write an ostinato

Give `userHarm` a **one-bar** pattern while the lead stays eight bars. Predict what
happens, then check it.

**Hand in:** what you predicted, what the `[audit]` line said, and what you heard.
When is a looping short voice a feature rather than a bug?

---

## Part D — Hardware **[hardware]**

### D1. Measure the real frame cost

Put a scope or logic analyser on SCL. Trigger on a display refresh and measure how
long the bus is busy for one full frame.

**Hand in:** the captured waveform, your measured figure, and a comparison with
the 23 ms the README calculates from 1024 bytes at 400 kHz. Account for the
difference.

### D2. Measure the jitter

Put the scope on the lead buzzer pin. Capture the interval between two known
consecutive note onsets, over several repeats of a track.

**Hand in:** your measured onset jitter, and a comparison with what `verify.py`
predicts. Does the slack-aware renderer hold up on real hardware?

### D3. Find the piezo's rolloff

Set `BASS_OCTAVE_SHIFT` to 0 and play Trap Arcade, whose bass sits in octave 2.
Then set it to 12 and play again.

**Hand in:** the two lowest notes you can actually hear from your buzzer, and an
explanation in terms of the mechanical resonance of a piezo element. At what
frequency does yours become useless as a bass?

### D4. Buzzer placement

Three elements summing acoustically is not a mixer. Try the buzzers touching, then
spread across the breadboard, then pointing in different directions.

**Hand in:** which arrangement gives the clearest separation between voices, and
why the answer is not simply "as far apart as possible."

---

## Part E — Open ended

Pick one:

- **E1.** Add a fourth voice. What has to change in `chiptune.h`, and what runs out
  first — LEDC channels, GPIO pins, or the ear's ability to separate square waves?
- **E2.** Replace the fixed duty per voice with an envelope: start a note at 50 %
  duty and decay it over the note's length. What does that do to the character of
  the sound, and what does it cost in `update()` time?
- **E3.** Drive the buzzers from a hardware timer interrupt instead of `loop()`.
  What does that buy, and what new hazards does it introduce around the shared
  player state?
- **E4.** Add a potentiometer on an ADC pin as a live tempo control. How do you
  change `_wholeUs` mid-track without every already-scheduled onset becoming wrong?
  (This one is harder than it looks — the absolute schedule is the thing that makes
  it hard.)

**Hand in:** working code, a short write-up of the design decision, and evidence
that it works — a `verify.py` run, a rendered WAV, or a scope capture.
