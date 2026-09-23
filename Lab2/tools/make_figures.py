#!/usr/bin/env python3
"""
make_figures.py - regenerate the report figures in ../figures.

Follows the course figure guide (references/ieee-figure-guide.md): dark
ink on white, sans-serif, ~200 DPI, no title baked into the image (the
caption lives in the document), and nothing that depends on colour to be
readable - every series is separated by fill texture and a direct label
as well, so the figures survive a grayscale print.

    python3 tools/make_figures.py            everything
    python3 tools/make_figures.py fig2 fig5  only these

Figures:
    fig1  system block diagram                     graphviz
    fig2  three-voice score timeline (piano roll)  matplotlib
    fig3  transport finite-state machine           graphviz
    fig4  loop() control flow                      graphviz
    fig5  measured note dropouts, two display
          scheduling policies                      matplotlib
    fig6  OLED screen layout, drawn at the
          sketch's real coordinates                PIL
"""

import os
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Patch
from PIL import Image, ImageDraw, ImageFont

import score
import verify

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
FIGS = os.path.join(ROOT, "figures")
DPI = 200

plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["DejaVu Sans", "Helvetica", "Arial"],
    "font.size": 8,
    "axes.linewidth": 0.8,
    "axes.edgecolor": "black",
    "xtick.color": "black",
    "ytick.color": "black",
    "text.color": "black",
    "axes.labelcolor": "black",
})

INK = "#111111"
MUTED = "#8a8a8a"


def run_dot(name, source):
    path = os.path.join(FIGS, name)
    p = subprocess.run(["dot", "-Tpng", "-Gdpi=200", "-o", path],
                       input=source, text=True, capture_output=True)
    if p.returncode:
        print(p.stderr, file=sys.stderr)
        raise SystemExit(f"graphviz failed for {name}")
    print("wrote", path)


# ======================================================================
def fig1():
    """System block diagram: input -> control -> output."""
    run_dot("fig1_block_diagram.png", r"""
digraph blocks {
  rankdir=LR;
  bgcolor="white";
  node [shape=box, style=rounded, fontname="Helvetica", fontsize=11,
        color="#111111", fontcolor="#111111", margin="0.14,0.09"];
  edge [fontname="Helvetica", fontsize=9, color="#111111",
        fontcolor="#111111"];

  subgraph cluster_in {
    label="INPUT"; fontname="Helvetica"; fontsize=10; color="#8a8a8a";
    style=dashed;
    btn [label="Momentary buttons x3\lNEXT  GPIO 32\lPLAY  GPIO 33\lMIX   GPIO 4\l(INPUT_PULLUP, active LOW)"];
  }

  subgraph cluster_mcu {
    label="CONTROL"; fontname="Helvetica"; fontsize=10; color="#8a8a8a";
    style=dashed;
    mcu [label="ESP32 DevKit-C V4\l\nFirmware:\l  debounce -> transport FSM\l  three-voice sequencer\l  (absolute us scheduling)\l  slack-aware renderer\l",
         style="rounded,bold"];
  }

  subgraph cluster_out {
    label="OUTPUT"; fontname="Helvetica"; fontsize=10; color="#8a8a8a";
    style=dashed;
    lead [label="Piezo - LEAD\lGPIO 27, LEDC ch A\l50 % duty\l"];
    harm [label="Piezo - HARMONY\lGPIO 26, LEDC ch B\l25 % duty\l"];
    bass [label="Piezo - BASS\lGPIO 25, LEDC ch C\l40 % duty\l"];
    oled [label="SSD1306 OLED 128x64\lI2C, addr 0x3C\lSDA 21 / SCL 22\l"];
  }

  pwr [label="USB 5 V\l3V3 regulator\l", shape=box, style="rounded,dashed"];

  pwr  -> mcu  [label="power"];
  btn  -> mcu  [label="digital in"];
  mcu  -> lead [label="PWM"];
  mcu  -> harm [label="PWM"];
  mcu  -> bass [label="PWM"];
  mcu  -> oled [label="I2C 400 kHz"];
  pwr  -> oled [label="3V3", style=dashed];
}
""")


# ======================================================================
def fig2():
    """Piano roll of the three voices - the figure that shows polyphony."""
    playlist, notes, cfg = score.default_playlist()
    title, bpm, voices = next(p for p in playlist if p[0] == "Trap Arcade")
    events, pass_us, _n, _t = score.timeline(voices, bpm, cfg, notes)

    bar_us = score.whole_us(bpm)
    window = 4 * bar_us            # first four bars
    ev = [e for e in events if e[1] < window]

    fig, ax = plt.subplots(figsize=(6.5, 2.9))

    # Identity by fill texture and a lane label, not by colour: this has
    # to survive a grayscale print.
    style = [
        dict(facecolor=INK,      edgecolor=INK, hatch=None, label="Lead (GPIO 27)"),
        dict(facecolor="#cccccc", edgecolor=INK, hatch="///", label="Harmony (GPIO 26)"),
        dict(facecolor="white",  edgecolor=INK, hatch=None, label="Bass (GPIO 25)"),
    ]

    def midi(f):
        import math
        return 69 + 12 * math.log2(f / 440.0)

    for v, onset, release, freq in ev:
        x = onset / 1e6
        w = (release - onset) / 1e6
        y = midi(freq)
        ax.add_patch(Rectangle((x, y - 0.42), w, 0.84,
                               linewidth=0.7, zorder=3, **{
                                   k: s for k, s in style[v].items()
                                   if k != "label"}))

    # bar lines
    for b in range(5):
        ax.axvline(b * bar_us / 1e6, color=MUTED, linewidth=0.6,
                   linestyle=(0, (4, 3)), zorder=1)
        if b < 4:
            ax.text(((b + 0.5) * bar_us) / 1e6, 96, f"bar {b+1}",
                    ha="center", va="top", fontsize=7, color=MUTED)

    ax.set_xlim(0, window / 1e6)
    ax.set_ylim(40, 97)
    ax.set_xlabel("time from start of track (s)")
    ax.set_ylabel("pitch (MIDI note number)")

    # octave gridlines, recessive
    for n in range(48, 97, 12):
        ax.axhline(n, color="#e2e2e2", linewidth=0.6, zorder=0)
    ax.set_yticks(range(48, 97, 12))
    ax.set_yticklabels([f"C{n//12 - 1}" for n in range(48, 97, 12)])

    ax.spines[["top", "right"]].set_visible(False)
    ax.legend(handles=[Patch(**s) for s in style], loc="upper right",
              frameon=False, fontsize=7, ncol=3,
              bbox_to_anchor=(1.0, 1.16))
    fig.tight_layout()
    path = os.path.join(FIGS, "fig2_voice_timeline.png")
    fig.savefig(path, dpi=DPI, facecolor="white")
    plt.close(fig)
    print("wrote", path, f"({title}, first 4 bars, {len(ev)} notes)")


# ======================================================================
def fig3():
    """Transport FSM."""
    run_dot("fig3_transport_fsm.png", r"""
digraph fsm {
  rankdir=LR; bgcolor="white";
  node [shape=circle, fontname="Helvetica", fontsize=10, fixedsize=true,
        width=1.05, color="#111111", fontcolor="#111111"];
  edge [fontname="Helvetica", fontsize=9, color="#111111",
        fontcolor="#111111"];

  start [shape=point, width=0.12, label=""];
  PLAYING [label="PLAYING"];
  PAUSED  [label="PAUSED"];
  ENDED   [label="ENDED", shape=doublecircle, width=1.0];

  start   -> PLAYING [label="setup()"];
  PLAYING -> PAUSED  [label="PLAY"];
  PAUSED  -> PLAYING [label="PLAY"];
  PLAYING -> ENDED   [label="elapsed >= total"];
  ENDED   -> PLAYING [label="PLAY (replay)\lor NEXT\lor auto-advance\lafter GAP_MS\l"];
  PLAYING -> PLAYING [label="NEXT (restart\lon next track)\l"];
  PAUSED  -> PLAYING [label="NEXT", style=dashed];
}
""")


# ======================================================================
def fig4():
    """loop() control flow, with the display's slack test called out."""
    run_dot("fig4_loop_flow.png", r"""
digraph flow {
  rankdir=TB; bgcolor="white"; ranksep=0.26; nodesep=0.28;
  size="6.4,8.2"; ratio=compress;
  node [fontname="Helvetica", fontsize=10, color="#111111",
        fontcolor="#111111"];
  edge [fontname="Helvetica", fontsize=9, color="#111111",
        fontcolor="#111111"];

  top   [shape=box, style=rounded, label="loop() entry\lnow = millis()\l"];
  btn   [shape=box, label="poll 3 buttons (debounced)\lNEXT / PLAY / MIX\l"];
  upd   [shape=box, style=bold,
         label="player.update(now)\lfor each voice:\l  release note if now >= offUs\l  while now >= nextUs: load next note\l"];
  endq  [shape=diamond, label="track\nfinished?", height=0.9, width=1.5];
  adv   [shape=box, label="auto-advance\lafter GAP_MS\l"];
  d1    [shape=diamond, label="since last draw\n>= DISPLAY_MIN_MS?", height=1.0, width=2.2];
  d2    [shape=diamond, label="slack >= 26 ms\nor forced?", height=1.0, width=1.9];
  draw  [shape=box, label="render + display()\l~23 ms blocking I2C\l"];
  skip  [shape=box, style=dashed, label="defer the frame\l(music wins)\l"];
  back  [shape=point, width=0.12, label=""];

  top -> btn -> upd -> endq;
  endq -> adv [label="yes"];
  endq -> d1  [label="no"];
  adv  -> d1;
  d1 -> d2   [label="yes"];
  d1 -> back [label="no"];
  d2 -> draw [label="yes"];
  d2 -> skip [label="no"];
  draw -> back;
  skip -> back;
  back -> top [label="repeat", constraint=false];
}
""")


# ======================================================================
def fig5():
    """
    Measured result: notes lost under two display-scheduling policies.

    Both bars come from running the real sequencer in tools/host_sim with
    the same 23 ms frame cost; only the decision of *when* to spend it
    differs.
    """
    playlist, notes, cfg = score.default_playlist()
    stall = 23

    def drops(mode):
        got, _log = verify.run_sim(late=stall if mode == "naive" else 0,
                                   frame=stall if mode == "slack" else 0)
        out = []
        for i, (title, bpm, voices) in enumerate(playlist):
            events, _p, _n, _t = score.timeline(voices, bpm, cfg, notes)
            exp, act = {}, {}
            for v, onset, _r, f in events:
                exp.setdefault(v, []).append((-(-onset // 1000), f))
            for v, t, f in got.get(i, []):
                act.setdefault(v, []).append((t, f))
            d = sum(verify.match(exp[v], act.get(v, []), stall + 3)[1]
                    for v in exp)
            out.append((title, d, sum(len(x) for x in exp.values())))
        return out

    naive = drops("naive")
    slack = drops("slack")

    titles = [t for t, _d, _n in naive]
    fig, ax = plt.subplots(figsize=(6.5, 2.8))
    x = range(len(titles))
    w = 0.38

    b1 = ax.bar([i - w / 2 for i in x], [d for _t, d, _n in naive], w,
                facecolor="#cccccc", edgecolor=INK, linewidth=0.8,
                hatch="///", label="Redraw on a timer", zorder=3)
    b2 = ax.bar([i + w / 2 for i in x], [d for _t, d, _n in slack], w,
                facecolor=INK, edgecolor=INK, linewidth=0.8,
                label="Redraw only when slack allows", zorder=3)

    for bars in (b1, b2):
        for r in bars:
            h = r.get_height()
            ax.text(r.get_x() + r.get_width() / 2, h + 0.35, f"{int(h)}",
                    ha="center", va="bottom", fontsize=7, color=INK)

    ax.set_xticks(list(x))
    ax.set_xticklabels(titles, rotation=20, ha="right", fontsize=7)
    ax.set_ylabel("notes never sounded")
    ax.set_ylim(0, max(4, max(d for _t, d, _n in naive) * 1.30))
    from matplotlib.ticker import MaxNLocator
    ax.yaxis.set_major_locator(MaxNLocator(integer=True))
    ax.yaxis.grid(True, color="#e2e2e2", linewidth=0.6, zorder=0)
    ax.set_axisbelow(True)
    ax.spines[["top", "right"]].set_visible(False)
    ax.legend(frameon=False, fontsize=7, loc="upper left")
    fig.tight_layout()
    path = os.path.join(FIGS, "fig5_display_scheduling.png")
    fig.savefig(path, dpi=DPI, facecolor="white")
    plt.close(fig)
    print("wrote", path)
    print("   naive:", {t: d for t, d, _ in naive if d})
    print("   slack:", {t: d for t, d, _ in slack if d} or "no drops")


# ======================================================================
def fig6():
    """
    The OLED screen, drawn at the coordinates the sketch actually uses.

    Adafruit_GFX's built-in font is a 5x7 glyph in a 6x8 cell, so every
    character is placed on a 6 px pitch here too. That makes this a real
    check of the layout, not an artist's impression: if a string runs
    past x=127 or two elements collide, it shows up here.
    """
    S = 6                       # display scale
    W, H = 128, 64
    img = Image.new("RGB", (W * S, H * S), "black")
    d = ImageDraw.Draw(img)
    try:
        font = ImageFont.load_default(size=int(5.4 * S))
    except TypeError:
        font = ImageFont.load_default()

    def text(col_x, row_y, s):
        """Draw on the 6x8 cell grid, one character at a time."""
        for i, ch in enumerate(s):
            d.text(((col_x + i * 6) * S, row_y * S), ch,
                   font=font, fill="white")

    def rect(x, y, w, h, fill=False):
        box = [x * S, y * S, (x + w) * S - 1, (y + h) * S - 1]
        if fill:
            d.rectangle(box, fill="white")
        else:
            d.rectangle(box, outline="white", width=max(1, S // 6))

    # --- the layout, mirroring renderNowPlaying() ---
    text(0, 0, "Trap Arcade")
    d.line([0, 9 * S, 127 * S, 9 * S], fill="white", width=max(1, S // 6))
    text(0, 12, "142 BPM")
    text(48, 12, "4/8")
    text(80, 12, "ALL")

    METER_X, METER_W, METER_H, Y0, STEP = 10, 80, 6, 22, 9
    for i, (tag, frac) in enumerate([("L", 0.78), ("H", 0.62), ("B", 0.21)]):
        y = Y0 + i * STEP
        text(0, y, tag)
        rect(METER_X, y, METER_W, METER_H)
        rect(METER_X + 1, y + 1, int((METER_W - 4) * frac) + 2, METER_H - 2,
             fill=True)

    # sprite
    sys.path.insert(0, HERE)
    import sprite_gen
    frames = [sprite_gen.FRAME_A, sprite_gen.FRAME_B, sprite_gen.FRAME_C]
    frames.append(sprite_gen.mirror(sprite_gen.FRAME_B))
    for yy, row in enumerate(frames[2]):
        for xx, c in enumerate(row):
            if c == "#":
                rect(102 + xx, 22 + yy, 1, 1, fill=True)

    rect(0, 48, 128, 7)
    rect(1, 49, int(126 * 0.43), 5, fill=True)
    text(0, 57, "0:23 / 0:54")
    text(78, 57, "x1/2")

    # annotation margin
    pad, top = 330, 8
    sheet = Image.new("RGB", (W * S + pad, H * S + 2 * top), "white")
    sheet.paste(img, (0, top))
    ad = ImageDraw.Draw(sheet)
    try:
        af = ImageFont.load_default(size=17)
    except TypeError:
        af = ImageFont.load_default()

    notes_ = [
        (3,  "track title"),
        (14, "tempo, track index, voice mix"),
        (24, "per-voice pitch meters:"),
        (31, "    L lead / H harmony / B bass"),
        (38, "    bar length is log(frequency)"),
        (45, "dancing sprite, one frame per beat"),
        (51, "elapsed / total progress"),
        (59, "time, and repeat count"),
    ]
    for y, label in notes_:
        ad.line([W * S + 4, y * S + top + 3, W * S + 16, y * S + top + 3],
                fill="#111111", width=2)
        ad.text((W * S + 22, y * S + top - 6), label, font=af,
                fill="#111111")

    path = os.path.join(FIGS, "fig6_oled_layout.png")
    sheet.save(path)
    print("wrote", path)


# ======================================================================
ALL = {"fig1": fig1, "fig2": fig2, "fig3": fig3,
       "fig4": fig4, "fig5": fig5, "fig6": fig6}

if __name__ == "__main__":
    os.makedirs(FIGS, exist_ok=True)
    wanted = sys.argv[1:] or list(ALL)
    for w in wanted:
        if w not in ALL:
            sys.exit(f"unknown figure {w}; choose from {', '.join(ALL)}")
        ALL[w]()
