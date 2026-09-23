# ENGR 4399 ST: Cyber-Physical & IoT Systems
## [Wokwi] Simulation Assignments — Fall 2026

**Course:** ENGR 4399 ST: Cyber-Physical & IoT Systems
**Instructor:** Dr. Okan Caglayan
**Student:** Daniel Critchlow Jr.
**Institution:** University of the Incarnate Word
**Term:** Fall 2026

---

### Background

This repository holds the ESP32 simulation assignments (SAR series) and the
in-class labs (Lab series) for ENGR 4399.
Each assignment recreates a project inspired by the ESP32 Super Starter Kit in the
[Wokwi](https://wokwi.com) online simulator, emphasizing 32-bit hardware–software
interaction, GPIO management, and professional engineering documentation under
version control.

Every folder is self-contained: firmware, the Wokwi schematic (`diagram.json`),
any screenshots or figures, and a README describing the design. Not every folder
has screenshots — each README says what evidence it does and does not carry.

---

### Simulation Assignments

| # | Project | Platform | Simulation |
|---|---------|----------|------------|
| [SAR 1](SAR1/) | ESP32 Push-Button RGB LED Color Cycler | Arduino (`.ino`) | [Wokwi](https://wokwi.com/projects/475932642565919745) |
| [SAR 2](SAR2/) | ESP32 Joystick-Controlled Servo | ESP-IDF (`.c`) + Arduino (`.ino`) | [Wokwi](https://wokwi.com/projects/475930488847754241) |

### In-Class Labs

Bench exercises built during class sessions. They are documented to the same
standard as the assignments but are not submitted for a grade, and they are not
always fully reproducible in the simulator — Lab 1, for example, uses a motor
driver that has no Wokwi part.

| # | Project | Platform | Simulation |
|---|---------|----------|------------|
| [Lab 1](Lab1/) | ESP32 DC Motor Controller with OLED Telemetry | Arduino (`.ino`) | Local build; Wokwi with substitute parts |
| [Lab 2](Lab2/) | ESP32 Three-Voice Chiptune Jukebox with OLED | Arduino (`.ino`) | Wokwi (`diagram.json`); sequencer also runs on a desktop |

---

### Repository Layout

```
ENGR-4399-ST-Cyber-Physical-IoT-Systems/
├── README.md
├── .gitignore
├── SAR1/                        Simulation Assignment 1
│   ├── README.md
│   ├── sketch/sketch.ino        Arduino firmware
│   ├── diagram.json             Wokwi schematic & wiring
│   ├── platformio.ini           PlatformIO build config
│   ├── wokwi.toml               Wokwi for VS Code config
│   ├── SAR1_Screenshot.png      Wokwi circuit view
│   ├── SAR1_ServiceFail.png     Build-queue error evidence
│   └── SAR1_Critchlow.docx      Submitted report
├── SAR2/                        Simulation Assignment 2
│   ├── README.md
│   ├── main/main.c              ESP-IDF firmware (reference)
│   ├── sketch.ino               Arduino-framework port
│   ├── CMakeLists.txt           ESP-IDF build files
│   ├── sdkconfig.defaults       Baseline ESP-IDF configuration
│   ├── wokwi.toml               Wokwi for VS Code config
│   ├── diagram.json             Wokwi schematic & wiring
│   ├── figures/                 IEEE-style report figures
│   └── SAR2_Critchlow.docx      Submitted report
├── Lab1/                        In-class lab 1
│   ├── README.md
│   ├── sketch/sketch.ino        Corrected Arduino firmware
│   ├── original/                The sketch exactly as run in class
│   ├── diagram.json             Wokwi schematic & wiring
│   ├── platformio.ini           PlatformIO build config
│   ├── wokwi.toml               Wokwi for VS Code config
│   └── figures/                 IEEE-style figures + generator script
└── Lab2/                        In-class lab 2
    ├── README.md
    ├── EXERCISES.md             Lab exercises
    ├── sketch/                  sketch.ino + engine, score, sprite headers
    ├── original/                The sketch exactly as run in class
    ├── diagram.json             Wokwi schematic & wiring
    ├── platformio.ini           PlatformIO build config
    ├── wokwi.toml               Wokwi for VS Code config
    ├── figures/                 IEEE-style figures
    └── tools/                   Score generators + desktop test harness
```

---

### Running a Simulation

**In the browser.** Open [wokwi.com](https://wokwi.com), start a new project for the
folder's platform — **ESP32 (Arduino)** for SAR 1 and Lab 1, **ESP32 (ESP-IDF)** for
SAR 2 — paste the firmware into the code tab and the folder's `diagram.json` into the
diagram tab, then press **Start**. The simulation links in the table above open the
published projects directly. Lab 1 additionally needs `SIM_WOKWI` set to 1, because
its motor driver and DHT11 have no Wokwi parts; its README lists the substitutions.

**Locally in VS Code.** Each assignment folder carries a `wokwi.toml` alongside its
`diagram.json`, so the **Wokwi for VS Code** extension can run it against a locally
compiled binary. Wokwi simulates firmware but does not build it, so compile first:

| Assignment | Toolchain | Build command | Artifacts |
|------------|-----------|---------------|-----------|
| SAR 1 | PlatformIO (Arduino) | `pio run` | `.pio/build/esp32dev/firmware.{bin,elf}` |
| SAR 2 | ESP-IDF | `idf.py build` | `build/flasher_args.json`, `build/sar2_joystick_servo.elf` |
| Lab 1 | PlatformIO (Arduino) | `pio run` | `.pio/build/esp32dev/firmware.{bin,elf}` |

Open the assignment folder as its own VS Code window — the extension looks for
`wokwi.toml` at the workspace root — build, then press `F1` →
**Wokwi: Start Simulator**. Build output is gitignored.

---

### Conventions

- One folder per assignment, named `SAR#`; in-class labs use `Lab#`.
- Firmware and `diagram.json` are the source of truth for every pin and value
  claimed in the corresponding report.
- Reports are submitted on Canvas in `.docx` format and mirrored here. In-class
  labs have no report; their README is the write-up.
- Instructor-provided assignment briefs, templates, and grading rubrics are
  intentionally excluded from this public repository.
