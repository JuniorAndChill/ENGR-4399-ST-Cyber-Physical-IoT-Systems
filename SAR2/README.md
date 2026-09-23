# SAR 2 — ESP32 Joystick-Controlled Servo

**Course:** ENGR 4399 ST: Cyber-Physical & IoT Systems
**Instructor:** Dr. Okan Caglayan
**Student:** Daniel Critchlow Jr.
**Date:** September 2026

---

## Project Overview

A two-axis analog joystick steers a hobby servo in real time on an ESP32 running
**ESP-IDF**. The X axis is sampled through the ESP32 ADC and mapped to a 0–180°
servo angle, which is driven by a 50 Hz LEDC PWM output. A center deadzone rejects
stick jitter, exponential smoothing turns the raw reading into a continuous sweep
instead of a snap, and pressing the joystick button recenters the horn to 90°.
Joystick values and the commanded angle are printed to the Serial Monitor about
five times per second.

## Hardware Components (Simulated)

* **Microcontroller:** Espressif ESP32 DevKit-C V4 (ESP-IDF build)
* **Input:** Two-axis analog joystick with push-select (`wokwi-analog-joystick`)
* **Output:** Hobby servo motor (`wokwi-servo`)

## Pin Map

| ESP32 Pin | Component | Function | Notes |
|-----------|-----------|----------|-------|
| GPIO 34 | Joystick HORZ | ADC1_CH6 input | X axis → servo angle; input-only pin |
| GPIO 35 | Joystick VERT | ADC1_CH7 input | Y axis → read and printed (spare) |
| GPIO 19 | Joystick SEL | Digital input | Internal pull-up, active LOW → recenter to 90° |
| GPIO 18 | Servo PWM | LEDC output | 50 Hz, 14-bit, 500–2500 µs pulse |
| 3V3 | Joystick VCC | Power | Joystick reference rail |
| 5V | Servo V+ | Power | Servo supply |
| GND | Joystick GND, Servo GND | Ground | Shared reference |

## Simulation Link

[Click here to view the Wokwi Simulation](https://wokwi.com/projects/475930488847754241)

## How It Works

**Servo drive.** `servo_init()` configures LEDC timer 0 in low-speed mode at 50 Hz
with 14-bit resolution (16 384 steps per 20 ms period). `servo_write_angle()` maps
an angle to a pulse width between `SERVO_MIN_PULSE_US` (500 µs, 0°) and
`SERVO_MAX_PULSE_US` (2500 µs, 180°), converts that pulse to a duty count, and
updates the channel.

**Joystick sampling.** `joystick_init()` brings up an ADC1 one-shot unit at 12-bit
width on both axes and configures GPIO 19 as an input with its internal pull-up.
`joystick_read()` averages eight samples per call to knock down ADC noise, and the
attenuation macro selects `ADC_ATTEN_DB_12` on ESP-IDF 5.2+ and `ADC_ATTEN_DB_11`
on older versions so the code builds across IDF releases.

**Angle mapping.** `axis_to_angle()` takes the raw reading's offset from the 2048
midpoint. Inside a ±150-count deadzone it returns exactly 90°, so a released stick
holds dead center. Outside the deadzone the offset is shifted by the deadzone width
before normalizing, which keeps the angle continuous at the deadzone edge instead
of jumping.

**Control loop.** `app_main()` runs every 20 ms. It reads both axes and the button,
picks a target angle (90° while pressed, otherwise the X-axis mapping), then applies
exponential smoothing — `angle += (target - angle) * 0.20` — so the horn sweeps
toward the target rather than snapping to it. Every tenth pass it prints the raw X
and Y values, the button state, and the commanded angle.

## Engineering Notes

* **Input-only pins.** GPIO 34 and 35 are input-only on the ESP32 and carry no
  internal pull-ups, which makes them a natural fit for analog axes and rules them
  out for the button — hence SEL on GPIO 19.
* **ADC1 vs ADC2.** Both axes use ADC1; ADC2 is shared with the Wi-Fi radio and
  becomes unavailable when Wi-Fi is active.
* **Resolution.** 14-bit LEDC resolution at 50 Hz gives roughly 1.2 µs per duty
  step, comfortably finer than the ~11 µs that corresponds to one degree of travel.

## Two Builds of the Same Design

The project is provided for both ESP32 toolchains. They share the same pin map,
deadzone, smoothing constant, and 50 Hz / 14-bit LEDC timing, so they drive the
servo identically.

| File | Framework | Notes |
|------|-----------|-------|
| `main/main.c` | ESP-IDF | Reference implementation; `adc_oneshot` + `ledc` drivers |
| `sketch.ino` | Arduino (arduino-esp32 3.x) | Functional port; uses `ledcAttach` / `ledcWrite` |

## Running the Simulation Locally (VS Code)

Requires the **Wokwi for VS Code** and **Espressif ESP-IDF** extensions. Wokwi
simulates a compiled binary — it does not build the code — so build first.

1. Open this `SAR2` folder as its own VS Code window (`File → Open Folder…`).
   The Wokwi extension reads `wokwi.toml` and `diagram.json` from the folder root.
2. Build: `idf.py set-target esp32` (first time only), then `idf.py build` — or use
   the ESP-IDF extension's **Build** button. This produces
   `build/flasher_args.json` and `build/sar2_joystick_servo.elf`, the paths
   `wokwi.toml` points at.
3. Press `F1` → **Wokwi: Start Simulator**.

Drag the on-canvas joystick to sweep the servo; click it to recenter. Serial
output appears in the simulator's terminal.

## Reproducing in the Browser

Open a new Wokwi project for the framework you want — **ESP32 (ESP-IDF)** for
`main/main.c`, **ESP32 (Arduino)** for `sketch.ino` — paste the firmware into the
code tab and `diagram.json` into the diagram tab, then press **Start**. The browser
ESP-IDF template supplies its own build files; the `CMakeLists.txt` files here are
for the local `idf.py` build.

## Files

| File | Description |
|------|-------------|
| `main/main.c` | ESP-IDF firmware (reference implementation) |
| `sketch.ino` | Arduino-framework port |
| `CMakeLists.txt`, `main/CMakeLists.txt` | ESP-IDF build files |
| `sdkconfig.defaults` | Baseline ESP-IDF configuration |
| `wokwi.toml` | Wokwi for VS Code simulation config |
| `diagram.json` | Wokwi schematic and wiring |
| `figures/` | IEEE-style report figures (block diagram, control flow, ADC transfer, PWM timing, smoothing response, simulation captures) |
| `SAR2_Critchlow.docx` | Submitted report |
| `wokwi-project.txt` | Wokwi project stub |
