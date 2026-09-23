# SAR 1 — ESP32 Push-Button RGB LED Color Cycler

**Course:** ENGR 4399 ST: Cyber-Physical & IoT Systems
**Instructor:** Dr. Okan Caglayan
**Student:** Daniel Critchlow Jr.
**Date:** September 2026

---

## Project Overview

A single momentary push button steps a common-cathode RGB LED through a fixed
four-color cycle: **Off → Red → Green → Blue → Off**. The ESP32 samples the button
on a GPIO configured with its internal pull-up, rejects contact bounce in software,
and drives the three LED channels as digital outputs from an explicit four-state
finite-state machine. The project demonstrates the complete input–process–output
chain of an embedded system with no external libraries.

## Hardware Components (Simulated)

* **Microcontroller:** Espressif ESP32 DevKit-C V4
* **Input:** Momentary push button (active-LOW, internal pull-up)
* **Output:** Common-cathode RGB LED through three series resistors

## Pin Map

| ESP32 Pin | Component | Direction | Notes |
|-----------|-----------|-----------|-------|
| GPIO 12 | Push button → GND | Digital input | `INPUT_PULLUP`; reads LOW when pressed |
| GPIO 25 | RGB LED — Red (via resistor) | Digital output | Red channel |
| GPIO 26 | RGB LED — Green (via resistor) | Digital output | Green channel |
| GPIO 27 | RGB LED — Blue (via resistor) | Digital output | Blue channel |
| GND | RGB common cathode, button return | — | Shared ground reference |

## Simulation Link

[Click here to view the Wokwi Simulation](https://wokwi.com/projects/475932642565919745)

The project also reproduces exactly from the files in this folder — open a new
Wokwi **ESP32 (Arduino)** project, paste `sketch/sketch.ino` into the code tab and
`diagram.json` into the diagram tab, then press **Start**.

## How It Works

`setup()` configures GPIO 12 as `INPUT_PULLUP`, GPIO 25/26/27 as outputs, and calls
`setColor(0, 0, 0)` so the system starts in a known Off state.

`loop()` reads the button every pass. Whenever the raw reading changes, a timestamp
is captured with `millis()`; the reading is only acted on once it has been stable
longer than `debounceDelay` (50 ms). Within that stable window a press is detected
as a HIGH-to-LOW edge (`reading == LOW && lastButtonState == HIGH`), which
guarantees exactly one action per physical press rather than repeated triggering
while the button is held.

Each accepted press advances the state with `colorState = (colorState + 1) % 4`,
and a `switch` maps the new state to a channel pattern through the `setColor()`
helper. Because the debounce uses `millis()` instead of `delay()`, the loop stays
non-blocking and the technique scales to systems with additional tasks.

## Verification

| # | Input | Expected | Result |
|---|-------|----------|--------|
| 1 | Power-on / reset | S0 — all channels off | Pass |
| 2 | Press once | S0→S1 — red | Pass |
| 3 | Press again | S1→S2 — green | Pass |
| 4 | Press again | S2→S3 — blue | Pass |
| 5 | Press again | S3→S0 — off (cycle wraps) | Pass |
| 6 | Mechanical bounce < 50 ms | Debounce suppresses; one advance | Pass |

## Known Limitations

* **Resistor value.** `diagram.json` specifies 220 kΩ series resistors — far above
  the 220–330 Ω normally used for LED current limiting. The Wokwi LED model still
  illuminates clearly, so the logic is fully demonstrable, but a physical build
  should use ~220–330 Ω per channel.
* **No Serial instrumentation.** The sketch emits no Serial output, so verification
  is visual rather than logged. Adding `Serial.begin(115200)` and printing each
  state transition would give timestamped evidence.
* **Build-queue availability.** Wokwi's shared free-tier build servers were
  intermittently unavailable during testing (`SAR1_ServiceFail.png`). This is an
  infrastructure limitation, not a design fault.

## Files

| File | Description |
|------|-------------|
| `sketch/sketch.ino` | Arduino firmware |
| `diagram.json` | Wokwi schematic and wiring |
| `SAR1_Screenshot.png` | Assembled Wokwi circuit |
| `SAR1_ServiceFail.png` | Build-servers-busy message |
| `SAR1_Critchlow.docx` | Submitted report |
