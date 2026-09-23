/************************************************************
  Lab 1 - ESP32 DC Motor Controller with OLED Telemetry
  ENGR 4399 ST: Cyber-Physical & IoT Systems
  Daniel Critchlow Jr. - Fall 2026
  ------------------------------------------------------------
  One cooperative loop() drives four subsystems at three
  different rates: an H-bridge DC motor (PWM), a three-page
  OLED dashboard, a DHT11 temperature/humidity sensor, and a
  PIR motion detector. Two debounced buttons are the only
  user input; an RGB LED mirrors the motor mode so the state
  is readable without the display.

  L293D motor driver (half-bridge pair, one motor):
      ENABLE (PWM / speed) -> GPIO 18
      DIRA   (In1)         -> GPIO 19
      DIRB   (In2)         -> GPIO 5

  OLED SSD1306 128x64 (I2C):
      SDA -> GPIO 21
      SCL -> GPIO 22
      Address 0x3C

  Controls (both active LOW, both on internal pull-ups):
      SWITCH button -> GPIO 25   OFF -> FWD -> REV -> SWEEP -> OFF
      PAGE   button -> GPIO 32   next OLED page

  RGB status LED (common cathode, HIGH = on) - one color per mode:
      RED   -> GPIO 26     OFF     = red
      GREEN -> GPIO 27     FORWARD = green
      BLUE  -> GPIO 33     REVERSE = blue
                           SWEEP   = magenta (red + blue)

  Sensors:
      DHT11 DATA -> GPIO 4
      PIR OUT    -> GPIO 16      (HC-SR501, 3.3 V logic)

  Libraries (Library Manager):
      - Adafruit GFX
      - Adafruit SSD1306
      - DHT sensor library (Adafruit) + Adafruit Unified Sensor
  Board: ESP32 Dev Module. Builds on arduino-esp32 2.x and 3.x - the PWM
  helper below selects the LEDC API that exists in the installed core.
************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// ---------- Target selection ----------
// Wokwi has no DHT11 part. The DHT11 and DHT22 use different data
// encodings, so the library type has to match the part that is fitted.
// Set SIM_WOKWI to 1 for the simulator build, 0 for the bench build. A build
// flag wins over this default, e.g. build_flags = -DSIM_WOKWI=1
#ifndef SIM_WOKWI
#define SIM_WOKWI 0
#endif

#if SIM_WOKWI
  #define DHTTYPE  DHT22
  #define DHT_NAME "DHT22"
#else
  #define DHTTYPE  DHT11
  #define DHT_NAME "DHT11"
#endif

// ---------- OLED ----------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_ADDR    0x3C
#define I2C_SDA        21
#define I2C_SCL        22
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Motor driver pins ----------
#define ENABLE     18   // L293D enable - PWM speed input
#define DIRA       19   // L293D In1   - direction
#define DIRB        5   // L293D In2   - direction

// ---------- Inputs ----------
#define SWITCH_BTN 25   // motor mode  (INPUT_PULLUP, active LOW)
#define PAGE_BTN   32   // OLED page   (INPUT_PULLUP, active LOW)
#define DHT_PIN     4   // DHT11 data
#define PIR_PIN    16   // HC-SR501 OUT

// ---------- RGB status LED (common cathode: HIGH = on) ----------
#define LED_R      26
#define LED_G      27
#define LED_B      33

DHT dht(DHT_PIN, DHTTYPE);

// ---------- PWM abstraction (arduino-esp32 2.x and 3.x) ----------
// analogWrite() was only added to the ESP32 core in 3.0.0; on 2.x the LEDC
// peripheral has to be driven directly. Both branches emit the same signal:
// 1 kHz carrier, 8-bit duty, which is what analogWrite() defaults to anyway.
#define PWM_FREQ_HZ 1000
#define PWM_BITS       8
#define PWM_CHANNEL    0            // LEDC channel, 2.x path only

#if defined(ESP_ARDUINO_VERSION) && defined(ESP_ARDUINO_VERSION_VAL) && \
    ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  #define PWM_CORE_3X 1
#else
  #define PWM_CORE_3X 0
#endif

void pwmInit(uint8_t pin) {
#if PWM_CORE_3X
  analogWrite(pin, 0);               // attaches the pin, 8-bit at 1 kHz
#else
  ledcSetup(PWM_CHANNEL, PWM_FREQ_HZ, PWM_BITS);
  ledcAttachPin(pin, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);
#endif
}

void pwmWrite(uint8_t pin, int duty) {
#if PWM_CORE_3X
  analogWrite(pin, duty);
#else
  (void)pin;
  ledcWrite(PWM_CHANNEL, duty);
#endif
}

// ---------- Motor modes ----------
enum MotorMode { MODE_OFF = 0, MODE_FWD, MODE_REV, MODE_SWEEP };
const int   NUM_MODES = 4;
const char *modeNames[NUM_MODES] = { "OFF", "FORWARD", "REVERSE", "SWEEP" };
MotorMode   motorMode = MODE_OFF;

// Duty used by the two fixed-speed modes. The PWM helper is 8-bit at
// 1 kHz, so 255 is 100 % duty.
// Lowering this constant lowers the running speed of FWD and REV.
const int RUN_DUTY = 255;

// ---------- OLED pages ----------
const int NUM_SCREENS = 3;
int currentScreen = 0;

// ---------- Sweep (PWM ramp) state ----------
int sweepSpeed = 0;   // 0..255 duty
int sweepStep  = 5;   // ramp increment per interval
unsigned long lastSweepUpdate = 0;
const unsigned long SWEEP_INTERVAL = 40;   // ms between ramp steps

// ---------- DHT read timing ----------
float tempC = NAN, tempF = NAN, humidity = NAN;
unsigned long lastDHTRead = 0;
// The DHT11 allows one reading per 2 s, and the Adafruit library re-serves
// its cached sample for any call inside 2000 ms. Polling at exactly 2000 ms
// can land a millisecond early and silently return the previous sample, so
// the period is set slightly longer.
const unsigned long DHT_INTERVAL = 2100;
int dhtFailCount = 0;                      // consecutive failed reads
const int DHT_FAIL_LIMIT = 5;              // warn only after this many

// ---------- PIR state ----------
bool motionActive = false;         // latched sensor output (no debounce)
bool motionEverSeen = false;
unsigned long lastMotionMs = 0;    // millis() of the last rising edge

// ---------- Display refresh timing ----------
unsigned long lastDisplay = 0;
const unsigned long DISPLAY_INTERVAL = 150;   // ~6.7 Hz

// ---------- Debounced, active-LOW button ----------
struct Button {
  uint8_t pin;
  bool    stableState;      // debounced level (HIGH = released)
  bool    lastReading;
  unsigned long lastChange;
};
const unsigned long DEBOUNCE_MS = 40;

void buttonInit(Button &b, uint8_t pin) {
  b.pin         = pin;
  b.stableState = HIGH;
  b.lastReading = HIGH;
  b.lastChange  = 0;
}

// Returns true exactly once per press, on the HIGH -> LOW edge.
bool buttonPressed(Button &b) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastReading) {
    b.lastChange  = millis();
    b.lastReading = reading;
  }
  if ((millis() - b.lastChange) > DEBOUNCE_MS && reading != b.stableState) {
    b.stableState = reading;
    if (b.stableState == LOW) return true;   // just pressed
  }
  return false;
}

Button switchBtn, pageBtn;

// ---------- RGB helper (common cathode: HIGH = on) ----------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_R, r ? HIGH : LOW);
  digitalWrite(LED_G, g ? HIGH : LOW);
  digitalWrite(LED_B, b ? HIGH : LOW);
}

// One distinct color per motor mode.
void applyModeColor() {
  switch (motorMode) {
    case MODE_OFF:   setRGB(1, 0, 0); break;   // red     - stopped
    case MODE_FWD:   setRGB(0, 1, 0); break;   // green   - forward
    case MODE_REV:   setRGB(0, 0, 1); break;   // blue    - reverse
    case MODE_SWEEP: setRGB(1, 0, 1); break;   // magenta - sweep
  }
}

// ---------- Apply motor outputs for the current mode ----------
// ENABLE is always written through pwmWrite(), so the pin keeps one
// owner for the life of the program. On arduino-esp32 3.x a pin that
// LEDC owns silently rejects digitalWrite(), so a mode that tried to
// stop the motor with digitalWrite(ENABLE, LOW) would instead leave it
// running at the last duty written.
void applyMotor() {
  switch (motorMode) {
    case MODE_FWD:
      digitalWrite(DIRA, HIGH);
      digitalWrite(DIRB, LOW);
      pwmWrite(ENABLE, RUN_DUTY);
      break;

    case MODE_REV:
      digitalWrite(DIRA, LOW);
      digitalWrite(DIRB, HIGH);
      pwmWrite(ENABLE, RUN_DUTY);
      break;

    case MODE_SWEEP:
      digitalWrite(DIRA, HIGH);          // sweep runs one direction
      digitalWrite(DIRB, LOW);
      pwmWrite(ENABLE, sweepSpeed);      // ramped in updateSweep()
      break;

    case MODE_OFF:
    default:
      pwmWrite(ENABLE, 0);               // coast: both outputs off
      digitalWrite(DIRA, LOW);
      digitalWrite(DIRB, LOW);
      break;
  }
  applyModeColor();
}

// ---------- Ramp the sweep duty up and down (non-blocking) ----------
// 255 / 5 = 51 steps at 40 ms = 2.04 s per leg, 4.08 s per cycle.
void updateSweep() {
  if (motorMode != MODE_SWEEP) return;
  if (millis() - lastSweepUpdate < SWEEP_INTERVAL) return;

  // Advance the deadline by one interval rather than re-basing it on the
  // service time, so a pass delayed by the DHT read or a frame push loses no
  // ramp time. If the backlog ever exceeds a few intervals, stop trying to
  // catch up so the ramp cannot run away.
  lastSweepUpdate += SWEEP_INTERVAL;
  if (millis() - lastSweepUpdate > 4 * SWEEP_INTERVAL) lastSweepUpdate = millis();

  sweepSpeed += sweepStep;
  if (sweepSpeed >= 255) { sweepSpeed = 255; sweepStep = -sweepStep; }
  if (sweepSpeed <= 0)   { sweepSpeed = 0;   sweepStep = -sweepStep; }
  pwmWrite(ENABLE, sweepSpeed);
}

// ---------- Read the PIR and latch edges (every pass) ----------
void updateMotion() {
  bool reading = (digitalRead(PIR_PIN) == HIGH);
  if (reading == motionActive) return;        // no edge, nothing to do

  motionActive = reading;
  if (motionActive) {
    lastMotionMs   = millis();
    motionEverSeen = true;
    Serial.println(F("PIR: motion detected"));
  } else {
    Serial.println(F("PIR: clear"));
  }
}

// ---------- Current duty for display ----------
int currentDuty() {
  switch (motorMode) {
    case MODE_FWD:
    case MODE_REV:   return RUN_DUTY;
    case MODE_SWEEP: return sweepSpeed;
    default:         return 0;
  }
}

// ==========================================================
//  OLED pages
// ==========================================================
void drawMotorScreen() {
  display.setCursor(0, 0);
  display.println(F("MOTOR CONTROL"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print(F("Mode : "));
  display.println(modeNames[motorMode]);

  display.setCursor(0, 28);
  display.print(F("Dir  : "));
  if (motorMode == MODE_FWD || motorMode == MODE_SWEEP) display.println(F("FWD >>"));
  else if (motorMode == MODE_REV)                       display.println(F("REV <<"));
  else                                                  display.println(F("--"));

  display.setCursor(0, 40);
  display.print(F("Duty : "));
  display.print(currentDuty());
  display.print(F("/255  "));
  display.print((currentDuty() * 100) / 255);
  display.println(F("%"));

  display.setCursor(0, 54);
  display.println(F("SWITCH = next mode"));
}

void drawEnvScreen() {
  display.setCursor(0, 0);
  display.println(F("ENVIRONMENT"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // A single dropped read keeps the last good values on screen; the
  // wait message only shows before the first successful reading.
  if (isnan(tempC) || isnan(humidity)) {
    display.setCursor(0, 14);
    display.print(F(DHT_NAME));
    display.println(F(": waiting"));
    display.setCursor(0, 24);
    display.println(F("check pin 4 wiring"));
  } else {
    display.setCursor(0, 14);
    display.print(F("Temp : "));
    display.print(tempC, 1);
    display.print(F(" C  "));
    display.print(tempF, 0);
    display.println(F("F"));

    display.setCursor(0, 24);
    display.print(F("Humid: "));
    display.print(humidity, 0);
    display.println(F(" %"));

    if (dhtFailCount >= DHT_FAIL_LIMIT) {
      display.setCursor(0, 34);
      display.println(F("(stale - check sens)"));
    }
  }

  display.setCursor(0, 44);
  display.print(F("Motion: "));
  display.println(motionActive ? F("ACTIVE") : F("clear"));

  display.setCursor(0, 54);
  if (!motionEverSeen) {
    display.println(F("Last  : --"));
  } else {
    display.print(F("Last  : "));
    display.print((millis() - lastMotionMs) / 1000);
    display.println(F(" s ago"));
  }
}

void drawInfoScreen() {
  display.setCursor(0, 0);
  display.println(F("SYSTEM INFO"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print(F("Page   : "));
  display.print(currentScreen + 1);
  display.print(F("/"));
  display.println(NUM_SCREENS);

  display.setCursor(0, 26);
  display.print(F("Uptime : "));
  display.print(millis() / 1000);
  display.println(F(" s"));

  display.setCursor(0, 36);
  display.print(F("Heap   : "));
  display.print(ESP.getFreeHeap() / 1024);
  display.println(F(" kB"));

  display.setCursor(0, 46);
  display.println(F("PAGE   = next page"));
  display.setCursor(0, 56);
  display.println(F("SWITCH = motor mode"));
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  switch (currentScreen) {
    case 0: drawMotorScreen(); break;
    case 1: drawEnvScreen();   break;
    case 2: drawInfoScreen();  break;
  }
  display.display();
}

// ==========================================================
void setup() {
  Serial.begin(115200);

  // Motor driver
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
  pwmInit(ENABLE);            // attach ENABLE to LEDC: 1 kHz, 8-bit, duty 0

  // Inputs - both buttons use the internal pull-up, so no external
  // resistor is needed on either one.
  pinMode(SWITCH_BTN, INPUT_PULLUP);
  pinMode(PAGE_BTN,   INPUT_PULLUP);
  pinMode(PIR_PIN,    INPUT);          // HC-SR501 drives the line both ways

  buttonInit(switchBtn, SWITCH_BTN);
  buttonInit(pageBtn,   PAGE_BTN);

  // RGB status LED
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setRGB(0, 0, 0);

  // I2C + OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed - halting"));
    for (;;) { delay(1000); }
  }

  dht.begin();

  // Splash
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("DC Motor + " DHT_NAME));
  display.println(F("ESP32 ready"));
  display.println(F("SWITCH = mode"));
  display.println(F("PAGE   = page"));
  display.display();
  delay(1500);

  applyMotor();               // enter MODE_OFF deliberately
  Serial.println(F("Lab 1 ready - mode: OFF"));
}

// ==========================================================
void loop() {
  // --- SWITCH: cycle motor mode ---
  if (buttonPressed(switchBtn)) {
    motorMode = (MotorMode)((motorMode + 1) % NUM_MODES);
    if (motorMode == MODE_SWEEP) {
      sweepSpeed = 0;
      sweepStep  = 5;
      lastSweepUpdate = millis();     // first step one full interval from now
    }
    applyMotor();
    Serial.print(F("Mode: "));
    Serial.println(modeNames[motorMode]);
  }

  // --- PAGE: flip OLED pages ---
  if (buttonPressed(pageBtn)) {
    currentScreen = (currentScreen + 1) % NUM_SCREENS;
    Serial.print(F("Page: "));
    Serial.println(currentScreen + 1);
    updateDisplay();                 // instant feedback on press
  }

  // --- Sweep ramp: every 40 ms while in SWEEP ---
  updateSweep();

  // --- PIR: every pass, acts only on edges ---
  updateMotion();

  // --- DHT: every 2 s ---
  if (millis() - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();        // Celsius
    if (!isnan(h) && !isnan(t)) {
      humidity = h;
      tempC    = t;
      tempF    = t * 9.0 / 5.0 + 32.0;
      dhtFailCount = 0;
    } else {
      dhtFailCount++;                       // keep last good values
      Serial.println(F("DHT read failed (transient)"));
    }
  }

  // --- OLED refresh: every 150 ms ---
  if (millis() - lastDisplay >= DISPLAY_INTERVAL) {
    lastDisplay = millis();
    updateDisplay();
  }
}
