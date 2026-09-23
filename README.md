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
| [SAR 1](SAR1/) | ESP32 Push-Button RGB LED Color Cycler | Arduino (`.ino`) | *link pending* |
| [SAR 2](SAR2/) | ESP32 Joystick-Controlled Servo | ESP-IDF (`.c`) | [Wokwi](https://wokwi.com/projects/475930488847754241) |

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
│   ├── SAR1_Screenshot.png      Wokwi circuit view
│   ├── SAR1_ServiceFail.png     Build-queue error evidence
│   └── SAR1_Critchlow.docx      Submitted report
└── SAR2/                        Simulation Assignment 2
    ├── README.md
    ├── main.c                   ESP-IDF firmware
    └── diagram.json             Wokwi schematic & wiring
```

---

### Reproducing a Simulation

1. Open [wokwi.com](https://wokwi.com) and start a new project for the assignment's
   platform — **ESP32 (Arduino)** for SAR 1, **ESP32 (ESP-IDF)** for SAR 2.
2. Paste the assignment's firmware (`sketch.ino` or `main.c`) into the code tab.
3. Paste the assignment's `diagram.json` into the diagram tab.
4. Press **Start** to compile and run.

---

### Conventions

- One folder per assignment, named `SAR#`.
- Firmware and `diagram.json` are the source of truth for every pin and value
  claimed in the corresponding report.
- Reports are submitted on Canvas in `.docx` format and mirrored here.
- Instructor-provided assignment briefs, templates, and grading rubrics are
  intentionally excluded from this public repository.
