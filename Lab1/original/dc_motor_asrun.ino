/************************************************************
  DC Motor + OLED + DHT11 control  (ESP32)
  --------------------------------------------------------
  L293D motor driver:
      ENABLE (PWM/speed) -> GPIO 18
      DIRA   (In1)       -> GPIO 19
      DIRB   (In2)       -> GPIO 5

  OLED SSD1306 128x64 (I2C):
      SDA -> GPIO 21
      SCL -> GPIO 22
      Addr 0x3C

  Controls:
      SWITCH button      -> GPIO 25  (INPUT_PULLUP, active LOW)
                            Cycles motor mode: OFF -> FWD -> REV -> SWEEP
      PAGE button        -> GPIO 35  (input-only pin, active LOW)
                            *** Needs an EXTERNAL ~10k pull-up to 3.3V ***
                            Flips through OLED screens.

  RGB status LED (common cathode, HIGH = ON) - one color per mode:
      RED   -> GPIO 26   OFF     = red
      GREEN -> GPIO 27   FORWARD = green
      BLUE  -> GPIO 33   REVERSE = blue
                         SWEEP   = magenta (red+blue)

  DHT11 temp/humidity:
      DATA -> GPIO 4

  Library needed (Library Manager):
      - Adafruit GFX
      - Adafruit SSD1306
      - DHT sensor library (Adafruit)  + Adafruit Unified Sensor
  Board: ESP32 (Arduino-ESP32 core 3.x recommended for analogWrite)
************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR    0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Pins ----------
#define ENABLE     18   // L293D enable (motor speed, PWM)
#define DIRA       19   // L293D In1 (direction)
#define DIRB        5   // L293D In2 (direction)

#define SWITCH_BTN 25   // mode button (INPUT_PULLUP, active LOW)
#define PAGE_BTN   35   // screen button (input-only, external pull-up, active LOW)
#define DHT_PIN     4   // DHT11 data

// ---------- RGB status LED (common cathode: HIGH = ON) ----------
#define LED_R      26   // red   (was the old single status LED)
#define LED_G      27   // green
#define LED_B      33   // blue

// ---------- DHT11 ----------
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// ---------- PIR ------------
int pirPin = 16; // Input for HC-S501
int pirValue; // Place to store read PIR Value

// ---------- Motor modes ----------
enum MotorMode { MODE_OFF = 0, MODE_FWD, MODE_REV, MODE_SWEEP };
const int NUM_MODES = 4;
const char* modeNames[NUM_MODES] = { "OFF", "FORWARD", "REVERSE", "SWEEP" };
int motorMode = MODE_OFF;

// ---------- Screens ----------
const int NUM_SCREENS = 3;
int currentScreen = 0;

// ---------- Sweep (PWM ramp) state ----------
int  sweepSpeed = 0;      // 0..255
int  sweepStep  = 5;      // ramp increment
unsigned long lastSweepUpdate = 0;
const unsigned long SWEEP_INTERVAL = 40;   // ms between ramp steps

// ---------- DHT read timing ----------
float tempC = NAN, tempF = NAN, humidity = NAN;
unsigned long lastDHTRead = 0;
const unsigned long DHT_INTERVAL = 2000;   // DHT11 max ~1 reading / 2s
int dhtFailCount = 0;                       // consecutive failed reads
const int DHT_FAIL_LIMIT = 5;              // only warn after this many in a row

// ---------- Display refresh timing ----------
unsigned long lastDisplay = 0;
const unsigned long DISPLAY_INTERVAL = 150;

// ---------- Simple debounced, active-LOW button ----------
struct Button {
  uint8_t pin;
  bool    stableState;      // debounced level (HIGH = released)
  bool    lastReading;
  unsigned long lastChange;
};
const unsigned long DEBOUNCE_MS = 40;

void buttonInit(Button &b, uint8_t pin) {
  b.pin = pin;
  b.stableState = HIGH;
  b.lastReading = HIGH;
  b.lastChange  = 0;
}

// returns true exactly once, on a fresh press (HIGH -> LOW)
bool buttonPressed(Button &b) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastReading) {
    b.lastChange = millis();
    b.lastReading = reading;
  }
  if ((millis() - b.lastChange) > DEBOUNCE_MS && reading != b.stableState) {
    b.stableState = reading;
    if (b.stableState == LOW) return true;   // just pressed
  }
  return false;
}

Button switchBtn, pageBtn;

// ---------- RGB helper (common cathode: HIGH = ON) ----------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_R, r ? HIGH : LOW);
  digitalWrite(LED_G, g ? HIGH : LOW);
  digitalWrite(LED_B, b ? HIGH : LOW);
}

// One distinct color per motor mode
void applyModeColor() {
  switch (motorMode) {
    case MODE_OFF:   setRGB(1, 0, 0); break;  // RED     - stopped
    case MODE_FWD:   setRGB(0, 1, 0); break;  // GREEN   - forward
    case MODE_REV:   setRGB(0, 0, 1); break;  // BLUE    - reverse
    case MODE_SWEEP: setRGB(1, 0, 1); break;  // MAGENTA - sweep
  }
}

// ---------- Apply motor outputs for the current mode ----------
void applyMotor() {
  switch (motorMode) {
    case MODE_FWD:
      // Match the proven Elegoo code exactly: enable HIGH, then direction.
      digitalWrite(ENABLE, HIGH);   // full on (NOT analogWrite)
      digitalWrite(DIRA, HIGH);     // one way
      digitalWrite(DIRB, LOW);
      break;
    case MODE_REV:
      digitalWrite(ENABLE, HIGH);   // full on
      digitalWrite(DIRA, LOW);      // reverse
      digitalWrite(DIRB, HIGH);
      break;
    case MODE_SWEEP:
      // Sweep is the only mode that truly needs PWM on the enable pin.
      digitalWrite(DIRA, HIGH);      // sweep runs one direction
      digitalWrite(DIRB, LOW);
      analogWrite(ENABLE, sweepSpeed); // updated in updateSweep()
      break;
    case MODE_OFF:
    default:
      digitalWrite(ENABLE, LOW);     // disable (also detaches PWM on ESP32)
      digitalWrite(DIRA, LOW);
      digitalWrite(DIRB, LOW);
      break;
  }
  // RGB status LED: one color per mode
  applyModeColor();
}

// ---------- Ramp the sweep speed up and down (non-blocking) ----------
void updateSweep() {
  if (motorMode != MODE_SWEEP) return;
  if (millis() - lastSweepUpdate < SWEEP_INTERVAL) return;
  lastSweepUpdate = millis();

  sweepSpeed += sweepStep;
  if (sweepSpeed >= 255) { sweepSpeed = 255; sweepStep = -sweepStep; }
  if (sweepSpeed <= 0)   { sweepSpeed = 0;   sweepStep = -sweepStep; }
  analogWrite(ENABLE, sweepSpeed);
}

// ---------- Current speed value for display ----------
int currentSpeed() {
  switch (motorMode) {
    case MODE_FWD:
    case MODE_REV:   return 255;
    case MODE_SWEEP: return sweepSpeed;
    default:         return 0;
  }
}

// ---------- Screens ----------
void drawMotorScreen() {
  display.setTextSize(1);
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
  display.print(F("Speed: "));
  display.print(currentSpeed());
  display.println(F(" /255"));

  display.setCursor(0, 54);
  display.println(F("SWITCH = next mode"));
}

void drawEnvScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("ENVIRONMENT"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Only show the warning if we have NEVER gotten a good reading.
  // A single dropped read keeps the last good values on screen.
  if (isnan(tempC) || isnan(humidity)) {
    display.setCursor(0, 24);
    display.println(F("DHT11: waiting..."));
    display.setCursor(0, 36);
    display.println(F("check pin 4 + pullup"));
    return;
  }

  display.setCursor(0, 18);
  display.print(F("Temp : "));
  display.print(tempC, 1);
  display.println(F(" C"));

  display.setCursor(0, 30);
  display.print(F("       "));
  display.print(tempF, 1);
  display.println(F(" F"));

  display.setCursor(0, 44);
  display.print(F("Humid: "));
  display.print(humidity, 0);
  display.println(F(" %"));

  // Small hint if reads have been failing for a while (values may be stale)
  if (dhtFailCount >= DHT_FAIL_LIMIT) {
    display.setCursor(0, 56);
    display.println(F("(stale - check sensor)"));
  }
}

void drawInfoScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("SYSTEM INFO"));
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print(F("Screen : "));
  display.print(currentScreen + 1);
  display.print(F("/"));
  display.println(NUM_SCREENS);

  display.setCursor(0, 28);
  display.print(F("Uptime : "));
  display.print(millis() / 1000);
  display.println(F(" s"));

  display.setCursor(0, 42);
  display.println(F("PAGE   = next screen"));
  display.setCursor(0, 54);
  display.println(F("SWITCH = motor mode"));
}

void updateDisplay() {
  display.clearDisplay();
  switch (currentScreen) {
    case 0: drawMotorScreen(); break;
    case 1: drawEnvScreen();   break;
    case 2: drawInfoScreen();  break;
  }
  display.display();
}

// ==========================================================
void setup() {
  Serial.begin(9600);

  // Motor pins
  pinMode(ENABLE, OUTPUT);
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  // Buttons
  pinMode(SWITCH_BTN, INPUT_PULLUP);   // active LOW
  pinMode(PAGE_BTN,   INPUT);          // GPIO35 input-only: external pull-up required

  // RGB LED
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setRGB(0, 0, 0);   // start off

  buttonInit(switchBtn, SWITCH_BTN);
  buttonInit(pageBtn,   PAGE_BTN);

  // I2C + OLED
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);   // stop here if the display is missing
  }

  // DHT11
  dht.begin();

  // PIR
  pinMode(pirPin, INPUT);

  // Splash
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Motor + DHT11"));
  display.println(F("ESP32 Ready"));
  display.println(F("SWITCH = mode"));
  display.println(F("PAGE   = screen"));
  display.display();
  delay(1500);

  applyMotor();   // start in OFF
}

// ==========================================================
void loop() {
  // --- SWITCH: cycle motor mode ---
  if (buttonPressed(switchBtn)) {
    motorMode = (motorMode + 1) % NUM_MODES;
    if (motorMode == MODE_SWEEP) { sweepSpeed = 0; sweepStep = 5; }
    applyMotor();
    Serial.print(F("Mode: "));
    Serial.println(modeNames[motorMode]);
  }

  // --- PAGE: flip screens ---
  if (buttonPressed(pageBtn)) {
    currentScreen = (currentScreen + 1) % NUM_SCREENS;
    Serial.print(F("Screen: "));
    Serial.println(currentScreen + 1);
    updateDisplay();   // instant feedback on press
  }

  // --- Sweep ramp (non-blocking) ---
  updateSweep();

  // ----PIR -------
  pirValue = digitalRead(pirPin);
  if (digitalRead(pirPin)){
    Serial.print("Motion");
  }

  // --- DHT11 read (every 2s) ---
  if (millis() - lastDHTRead >= DHT_INTERVAL) {
    lastDHTRead = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();      // Celsius
    if (!isnan(h) && !isnan(t)) {
      humidity = h;
      tempC = t;
      tempF = t * 9.0 / 5.0 + 32.0;
      dhtFailCount = 0;                    // good read, reset counter
    } else {
      dhtFailCount++;                      // keep last good values on screen
      Serial.println(F("DHT read failed (transient)"));
    }
  }

  // --- Refresh OLED periodically ---
  if (millis() - lastDisplay >= DISPLAY_INTERVAL) {
    lastDisplay = millis();
    updateDisplay();
  }
}