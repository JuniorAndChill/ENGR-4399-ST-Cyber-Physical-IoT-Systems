# ENGR 4399 ST: Cyber-Physical & IoT Systems
## [Wokwi] Simulation Assignments — Fall 2026

**Course:** ENGR 4399 ST: Cyber-Physical & IoT Systems
**Instructor:** Dr. Okan Caglayan
**Student:** Daniel Critchlow Jr.
**Institution:** University of the Incarnate Word
**Term:** Fall 2026

---

### Background

This repository holds the ESP32 simulation assignments (SAR series) for ENGR 4399.
Each assignment recreates a project inspired by the ESP32 Super Starter Kit in the
[Wokwi](https://wokwi.com) online simulator, emphasizing 32-bit hardware–software
interaction, GPIO management, and professional engineering documentation under
version control.

Every assignment folder is self-contained: firmware, the Wokwi schematic
(`diagram.json`), screenshots, and a README describing the design.

---

### Assignments

| # | Project | Platform | Simulation |
|---|---------|----------|------------|
| [SAR 1](SAR1/) | ESP32 Push-Button RGB LED Color Cycler | Arduino (`.ino`) | [Wokwi](https://wokwi.com/projects/475932642565919745) |
| [SAR 2](SAR2/) | ESP32 Joystick-Controlled Servo | ESP-IDF (`.c`) + Arduino (`.ino`) | [Wokwi](https://wokwi.com/projects/475930488847754241) |

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
└── SAR2/                        Simulation Assignment 2
    ├── README.md
    ├── main/main.c              ESP-IDF firmware (reference)
    ├── sketch.ino               Arduino-framework port
    ├── CMakeLists.txt           ESP-IDF build files
    ├── sdkconfig.defaults       Baseline ESP-IDF configuration
    ├── wokwi.toml               Wokwi for VS Code config
    ├── diagram.json             Wokwi schematic & wiring
    ├── figures/                 IEEE-style report figures
    └── SAR2_Critchlow.docx      Submitted report
```

---

### Running a Simulation

**In the browser.** Open [wokwi.com](https://wokwi.com), start a new project for the
assignment's platform — **ESP32 (Arduino)** for SAR 1, **ESP32 (ESP-IDF)** for SAR 2
— paste the firmware into the code tab and the assignment's `diagram.json` into the
diagram tab, then press **Start**. The simulation links in the table above open the
published projects directly.

**Locally in VS Code.** Each assignment folder carries a `wokwi.toml` alongside its
`diagram.json`, so the **Wokwi for VS Code** extension can run it against a locally
compiled binary. Wokwi simulates firmware but does not build it, so compile first:

| Assignment | Toolchain | Build command | Artifacts |
|------------|-----------|---------------|-----------|
| SAR 1 | PlatformIO (Arduino) | `pio run` | `.pio/build/esp32dev/firmware.{bin,elf}` |
| SAR 2 | ESP-IDF | `idf.py build` | `build/flasher_args.json`, `build/sar2_joystick_servo.elf` |

Open the assignment folder as its own VS Code window — the extension looks for
`wokwi.toml` at the workspace root — build, then press `F1` →
**Wokwi: Start Simulator**. Build output is gitignored.

---

### Conventions

- One folder per assignment, named `SAR#`.
- Firmware and `diagram.json` are the source of truth for every pin and value
  claimed in the corresponding report.
- Reports are submitted on Canvas in `.docx` format and mirrored here.
- Instructor-provided assignment briefs, templates, and grading rubrics are
  intentionally excluded from this public repository.
