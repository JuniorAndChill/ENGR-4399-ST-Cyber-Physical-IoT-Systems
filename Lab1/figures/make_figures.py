#!/usr/bin/env python3
"""
Regenerate the IEEE-style figures for Lab 1 (ESP32 DC motor controller).

    cd Lab1/figures && python3 make_figures.py

Requirements: graphviz ("dot" on PATH) and matplotlib. Every figure is drawn in
black on white with distinct line styles rather than colour, so it survives
grayscale printing. Captions live in the report/README, not in the images.
"""

import os
import subprocess
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

HERE = os.path.dirname(os.path.abspath(__file__))
DPI = 200
FONT = "Helvetica"

plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["DejaVu Sans"],
    "font.size": 9,
    "axes.linewidth": 0.9,
    "axes.edgecolor": "black",
    "savefig.bbox": "tight",
    "savefig.pad_inches": 0.06,
})


def render_dot(name, source):
    """Write a .dot file next to the figures and render it to PNG at 200 dpi."""
    dot_path = os.path.join(HERE, name + ".dot")
    png_path = os.path.join(HERE, name + ".png")
    with open(dot_path, "w") as fh:
        fh.write(source)
    subprocess.run(["dot", "-Tpng", "-Gdpi=%d" % DPI, dot_path, "-o", png_path],
                   check=True)
    print("wrote", os.path.basename(png_path))


# ----------------------------------------------------------------------
# Figure 1 - system block diagram
# ----------------------------------------------------------------------
FIG1 = """
digraph blocks {
  rankdir=LR;
  bgcolor="white";
  fontname="%(f)s";
  nodesep=0.35; ranksep=0.75;
  node [shape=box, style="rounded", fontname="%(f)s", fontsize=10,
        color=black, penwidth=1.1, margin="0.12,0.08"];
  edge [fontname="%(f)s", fontsize=9, color=black, penwidth=1.0,
        arrowsize=0.7];

  subgraph cluster_in {
    label="INPUT"; fontname="%(f)s"; fontsize=10; style="dashed"; color="gray40";
    sw   [label="SWITCH push button\\nGPIO 25, INPUT_PULLUP\\nactive LOW"];
    pg   [label="PAGE push button\\nGPIO 32, INPUT_PULLUP\\nactive LOW"];
    dht  [label="DHT11\\ntemperature / humidity\\nGPIO 4, single-wire"];
    pir  [label="PIR HC-SR501\\nmotion detector\\nGPIO 16, digital"];
  }

  mcu [label="ESP32 DevKit-C V4\\n\\n40 ms  PWM ramp\\n150 ms  display refresh\\n2100 ms  sensor read\\ncooperative loop()",
       style="rounded,bold", penwidth=2.0];

  subgraph cluster_out {
    label="OUTPUT"; fontname="%(f)s"; fontsize=10; style="dashed"; color="gray40";
    drv  [label="L293D half-bridge\\nEN  GPIO 18 (PWM, 1 kHz)\\nIn1 GPIO 19   In2 GPIO 5"];
    mot  [label="DC motor", shape=ellipse, style="solid"];
    oled [label="SSD1306 OLED 128x64\\nI2C 0x3C\\nSDA GPIO 21, SCL GPIO 22"];
    rgb  [label="RGB LED, common cathode\\nGPIO 26 / 27 / 33\\nvia 220 ohm"];
  }

  pwr [label="USB 5 V\\non-board 3V3 regulator", shape=box, style="dashed"];

  sw  -> mcu [label="debounced edge"];
  pg  -> mcu [label="debounced edge"];
  dht -> mcu [label="serial frame"];
  pir -> mcu [label="level"];

  mcu -> drv  [label="duty + direction"];
  drv -> mot  [label="bridge output"];
  mcu -> oled [label="I2C frames"];
  mcu -> rgb  [label="digital out"];

  pwr -> mcu [style=dashed];
  pwr -> drv [style=dashed];
}
""" % {"f": FONT}


# ----------------------------------------------------------------------
# Figure 2 - motor-mode finite-state machine
# ----------------------------------------------------------------------
FIG2 = """
digraph motor_fsm {
  rankdir=LR;
  bgcolor="white";
  fontname="%(f)s";
  nodesep=0.55;
  node [shape=circle, fixedsize=true, width=1.35, fontname="%(f)s",
        fontsize=9, color=black, penwidth=1.1];
  edge [fontname="%(f)s", fontsize=9, color=black, penwidth=1.0, arrowsize=0.7];

  start [shape=point, width=0.12, label=""];
  S0 [label="S0  OFF\\nEN duty 0\\nIn1=0 In2=0\\nLED red", shape=doublecircle, width=1.30];
  S1 [label="S1  FORWARD\\nEN duty 255\\nIn1=1 In2=0\\nLED green"];
  S2 [label="S2  REVERSE\\nEN duty 255\\nIn1=0 In2=1\\nLED blue"];
  S3 [label="S3  SWEEP\\nEN duty ramp\\nIn1=1 In2=0\\nLED magenta"];

  start -> S0 [label="reset"];
  S0 -> S1 [label="SWITCH"];
  S1 -> S2 [label="SWITCH"];
  S2 -> S3 [label="SWITCH"];
  S3 -> S0 [label="SWITCH", constraint=false];
}
""" % {"f": FONT}


# ----------------------------------------------------------------------
# Figure 3 - OLED page state machine
# ----------------------------------------------------------------------
FIG3 = """
digraph pages {
  rankdir=LR;
  bgcolor="white";
  fontname="%(f)s";
  nodesep=0.6;
  node [shape=box, style="rounded", fontname="%(f)s", fontsize=9,
        color=black, penwidth=1.1, margin="0.14,0.10"];
  edge [fontname="%(f)s", fontsize=9, color=black, penwidth=1.0, arrowsize=0.7];

  start [shape=point, width=0.12, label=""];
  P0 [label="Page 1  MOTOR CONTROL\\nmode, direction,\\nduty / 255 and percent", style="rounded,bold"];
  P1 [label="Page 2  ENVIRONMENT\\ntemp C and F, humidity,\\nmotion state, time since"];
  P2 [label="Page 3  SYSTEM INFO\\npage index, uptime,\\nfree heap, key legend"];

  start -> P0 [label="reset"];
  P0 -> P1 [label="PAGE"];
  P1 -> P2 [label="PAGE"];
  P2 -> P0 [label="PAGE", constraint=false];
}
""" % {"f": FONT}


# ----------------------------------------------------------------------
# Figure 4 - firmware control flow
# ----------------------------------------------------------------------
FIG4 = """
digraph flow {
  bgcolor="white";
  fontname="%(f)s";
  ranksep=0.28; nodesep=0.40;
  node [shape=box, style="rounded", fontname="%(f)s", fontsize=9,
        color=black, penwidth=1.1, margin="0.12,0.07"];
  edge [fontname="%(f)s", fontsize=8.5, color=black, penwidth=1.0, arrowsize=0.65];

  setup [label="setup()\\nGPIO + LEDC attach, I2C, OLED,\\nDHT begin, splash 1.5 s,\\nenter S0 OFF", style="rounded,bold"];

  b1 [label="SWITCH pressed?\\n(40 ms debounce)", shape=diamond, style="", width=1.9, height=0.9];
  a1 [label="mode = (mode+1) mod 4\\nreset ramp on entry to SWEEP\\napplyMotor()"];

  b2 [label="PAGE pressed?\\n(40 ms debounce)", shape=diamond, style="", width=1.9, height=0.9];
  a2 [label="page = (page+1) mod 3\\nredraw immediately"];

  b3 [label="SWEEP and\\n40 ms elapsed?", shape=diamond, style="", width=1.9, height=0.9];
  a3 [label="duty += step, fold at 0 and 255\\nanalogWrite(EN, duty)"];

  b4 [label="PIR level changed?", shape=diamond, style="", width=1.9, height=0.8];
  a4 [label="latch state, stamp millis()\\nprint the edge once"];

  b5 [label="2100 ms elapsed?", shape=diamond, style="", width=1.9, height=0.8];
  a5 [label="read humidity + temperature\\ngood: store, clear fail count\\nbad: keep last values, count++"];

  b6 [label="150 ms elapsed?", shape=diamond, style="", width=1.9, height=0.8];
  a6 [label="clear, draw active page,\\npush frame over I2C"];

  loop [label="end of loop()  ->  repeat", shape=box, style="dashed"];

  setup -> b1;
  b1 -> a1 [label="yes"]; a1 -> b2;
  b1 -> b2 [label="no"];
  b2 -> a2 [label="yes"]; a2 -> b3;
  b2 -> b3 [label="no"];
  b3 -> a3 [label="yes"]; a3 -> b4;
  b3 -> b4 [label="no"];
  b4 -> a4 [label="yes"]; a4 -> b5;
  b4 -> b5 [label="no"];
  b5 -> a5 [label="yes"]; a5 -> b6;
  b5 -> b6 [label="no"];
  b6 -> a6 [label="yes"]; a6 -> loop;
  b6 -> loop [label="no"];
}
""" % {"f": FONT}


# ----------------------------------------------------------------------
# Figure 5 - sweep duty ramp
# ----------------------------------------------------------------------
def fig5_sweep_ramp():
    interval_ms = 40
    duty, step = 0, 5
    t_ms, duties = [], []
    for i in range(230):                  # 230 * 40 ms = 9.2 s
        t_ms.append(i * interval_ms)
        duties.append(duty)
        duty += step
        if duty >= 255:
            duty, step = 255, -step
        if duty <= 0:
            duty, step = 0, -step

    t_s = [t / 1000.0 for t in t_ms]
    fig, ax = plt.subplots(figsize=(6.5, 2.9))
    ax.step(t_s, duties, where="post", color="black", linewidth=1.2)

    ax.axhline(255, color="gray", linewidth=0.7, linestyle=":")
    for x in (0.0, 2.04, 4.08, 6.12, 8.16):
        ax.axvline(x, color="gray", linewidth=0.7, linestyle=":")

    ax.annotate("", xy=(0.0, 272), xytext=(4.08, 272),
                arrowprops=dict(arrowstyle="<->", color="black", linewidth=0.9))
    ax.text(2.04, 278, "one cycle = 102 x 40 ms = 4.08 s",
            ha="center", va="bottom", fontsize=8.5)
    ax.text(1.02, 120, "51 steps up\n+5 per 40 ms", ha="center", fontsize=8.5)
    ax.text(3.06, 120, "51 steps down\n-5 per 40 ms", ha="center", fontsize=8.5)

    ax.set_xlabel("time in SWEEP mode (s)")
    ax.set_ylabel("analogWrite duty (8-bit)")
    ax.set_xlim(0, 9.2)
    ax.set_ylim(0, 300)
    ax.set_yticks([0, 64, 128, 192, 255])
    ax.xaxis.set_major_locator(MultipleLocator(1.0))
    ax.grid(axis="y", color="gray", linewidth=0.4, alpha=0.35)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    out = os.path.join(HERE, "fig5_sweep_ramp.png")
    fig.savefig(out, dpi=DPI)
    plt.close(fig)
    print("wrote", os.path.basename(out))


# ----------------------------------------------------------------------
# Figure 6 - PWM duty at the three operating points
# ----------------------------------------------------------------------
def fig6_pwm_duty():
    period_ms = 1.0                        # analogWrite default 1 kHz
    cycles = 3
    cases = [(64, "duty 64 / 255  (25 %)", "-"),
             (128, "duty 128 / 255  (50 %)", "-"),
             (255, "duty 255 / 255  (100 %)", "-")]

    fig, axes = plt.subplots(len(cases), 1, figsize=(6.5, 3.6), sharex=True)
    for ax, (duty, label, ls) in zip(axes, cases):
        frac = duty / 255.0
        xs, ys = [], []
        if frac >= 1.0:
            xs, ys = [0.0, cycles * period_ms], [1, 1]      # no falling edge
        else:
            for c in range(cycles):
                t0 = c * period_ms
                xs += [t0, t0 + frac * period_ms,
                       t0 + frac * period_ms, t0 + period_ms]
                ys += [1, 1, 0, 0]
        ax.plot(xs, ys, color="black", linewidth=1.3, linestyle=ls,
                drawstyle="steps-post")
        ax.fill_between(xs, 0, ys, step="post", color="black", alpha=0.10)
        ax.set_ylim(-0.25, 1.45)
        ax.set_yticks([0, 1])
        ax.set_yticklabels(["0 V", "3.3 V"])
        ax.text(cycles * period_ms - 0.02, 1.18, label, fontsize=8.5,
                va="center", ha="right")
        ax.grid(axis="x", color="gray", linewidth=0.4, alpha=0.35)
        ax.spines["top"].set_visible(False)
        ax.spines["right"].set_visible(False)

    axes[0].annotate("", xy=(0, 1.34), xytext=(1.0, 1.34),
                     arrowprops=dict(arrowstyle="<->", color="black", linewidth=0.8))
    axes[0].text(0.5, 1.37, "T = 1 ms (1 kHz default)", ha="center",
                 va="bottom", fontsize=8)
    axes[-1].set_xlabel("time (ms)")
    axes[-1].set_xlim(0, cycles * period_ms)
    fig.text(0.015, 0.55, "L293D ENABLE (GPIO 18)", rotation=90,
             va="center", fontsize=9)
    fig.subplots_adjust(left=0.13, hspace=0.35)
    out = os.path.join(HERE, "fig6_pwm_duty.png")
    fig.savefig(out, dpi=DPI)
    plt.close(fig)
    print("wrote", os.path.basename(out))


# ----------------------------------------------------------------------
# Figure 7 - cooperative task cadence
# ----------------------------------------------------------------------
def fig7_task_cadence():
    span_ms = 2400
    tasks = [
        ("buttons + PIR\nevery pass", None),
        ("PWM ramp\n40 ms", 40),
        ("OLED refresh\n150 ms", 150),
        ("DHT read\n2100 ms", 2100),
    ]
    fig, ax = plt.subplots(figsize=(6.5, 2.6))
    for row, (label, period) in enumerate(tasks):
        y = len(tasks) - 1 - row
        ax.hlines(y, 0, span_ms, color="gray", linewidth=0.6, linestyle=":")
        if period is None:
            ax.hlines(y, 0, span_ms, color="black", linewidth=4.0, alpha=0.30)
        else:
            marks = list(range(0, span_ms + 1, period))
            ax.vlines(marks, y - 0.22, y + 0.22, color="black", linewidth=1.0)
        ax.text(-60, y, label, ha="right", va="center", fontsize=8.5)

    ax.set_xlim(0, span_ms)
    ax.set_ylim(-0.6, len(tasks) - 0.4)
    ax.set_yticks([])
    ax.set_xlabel("time (ms)")
    ax.xaxis.set_major_locator(MultipleLocator(300))
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    out = os.path.join(HERE, "fig7_task_cadence.png")
    fig.savefig(out, dpi=DPI)
    plt.close(fig)
    print("wrote", os.path.basename(out))


if __name__ == "__main__":
    render_dot("fig1_block_diagram", FIG1)
    render_dot("fig2_motor_fsm", FIG2)
    render_dot("fig3_page_fsm", FIG3)
    render_dot("fig4_control_flow", FIG4)
    fig5_sweep_ramp()
    fig6_pwm_duty()
    fig7_task_cadence()
