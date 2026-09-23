// Joystick-Controlled Servo — Arduino-framework port (ESP32)
// ENGR 4399 ST: Cyber-Physical and IoT Systems — SAR 2
// Daniel Critchlow Jr.
//
// Functionally equivalent to the ESP-IDF reference implementation in main.c.
// Same pin map, same dead zone, same smoothing constant, same 50 Hz / 14-bit
// LEDC timing, so both builds drive the servo identically.
//
// Written for the arduino-esp32 3.x core (ledcAttach / ledcWrite take a pin).
// On a 2.x core, replace ledcAttach() with:
//     ledcSetup(0, SERVO_FREQ_HZ, SERVO_RES_BITS);
//     ledcAttachPin(SERVO_GPIO, 0);
// and pass the channel number (0) to ledcWrite() instead of the pin.
//
// Wiring (see diagram.json):
//   Joystick HORZ -> GPIO34  (ADC1_CH6)   X axis -> servo angle
//   Joystick VERT -> GPIO35  (ADC1_CH7)   Y axis -> read & printed (spare)
//   Joystick SEL  -> GPIO19  (active LOW) press  -> recenter to 90 deg
//   Servo PWM     -> GPIO18  (LEDC channel 0, 50 Hz)

// ---------------- configuration ----------------
const int SERVO_GPIO   = 18;
const int JOY_SEL_GPIO = 19;
const int JOY_X_PIN    = 34;          // ADC1_CH6
const int JOY_Y_PIN    = 35;          // ADC1_CH7

const int   SERVO_MIN_PULSE_US = 500;    // 0 deg
const int   SERVO_MAX_PULSE_US = 2500;   // 180 deg
const int   SERVO_FREQ_HZ      = 50;     // 20 ms period
const int   SERVO_RES_BITS     = 14;     // 16384 steps per period

const int   ADC_CENTER   = 2048;
const int   ADC_DEADZONE = 150;          // ignore jitter around center
const float SMOOTHING    = 0.20f;        // 0..1, lower = smoother
const int   LOOP_PERIOD_MS = 20;

float angle = 90.0f;                     // current commanded angle
int   tick  = 0;

// ---------------- servo ----------------
// Convert an angle to a pulse width, then to an LEDC duty count.
// duty = pulse_us * freq * 2^res / 1e6
void servoWriteAngle(float a) {
  if (a < 0.0f)   a = 0.0f;
  if (a > 180.0f) a = 180.0f;

  float pulseUs = SERVO_MIN_PULSE_US +
                  (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US) * (a / 180.0f);

  uint32_t maxDuty = (1UL << SERVO_RES_BITS);          // 16384 steps per period
  uint32_t duty    = (uint32_t)((pulseUs * SERVO_FREQ_HZ * maxDuty) / 1000000.0f);

  ledcWrite(SERVO_GPIO, duty);
}

// ---------------- joystick ----------------
// Average 8 samples to knock down ADC noise.
int joystickRead(int pin) {
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(pin);
  return (int)(sum / 8);
}

// Raw ADC -> 0..180 deg, with a dead zone that holds dead centre at 90 deg.
float axisToAngle(int raw) {
  int offset = raw - ADC_CENTER;
  if (abs(offset) < ADC_DEADZONE) return 90.0f;

  // Re-scale so the angle is continuous at the edge of the dead zone.
  if (offset > 0) offset -= ADC_DEADZONE;
  else            offset += ADC_DEADZONE;

  float span = (float)(ADC_CENTER - ADC_DEADZONE);   // 1898
  float norm = (float)offset / span;                 // -1 .. +1
  if (norm < -1.0f) norm = -1.0f;
  if (norm >  1.0f) norm =  1.0f;

  return 90.0f + norm * 90.0f;
}

// ---------------- setup / loop ----------------
void setup() {
  Serial.begin(115200);

  ledcAttach(SERVO_GPIO, SERVO_FREQ_HZ, SERVO_RES_BITS);   // 50 Hz, 14-bit

  analogReadResolution(12);                    // 0..4095
  analogSetPinAttenuation(JOY_X_PIN, ADC_11db);
  analogSetPinAttenuation(JOY_Y_PIN, ADC_11db);

  pinMode(JOY_SEL_GPIO, INPUT_PULLUP);         // idle HIGH, pressed LOW

  Serial.println("Joystick -> servo ready. Move the stick; press it to recenter.");

  servoWriteAngle(angle);                      // home the horn at 90 deg
  delay(300);
}

void loop() {
  int  xRaw    = joystickRead(JOY_X_PIN);
  int  yRaw    = joystickRead(JOY_Y_PIN);
  bool pressed = (digitalRead(JOY_SEL_GPIO) == LOW);

  float target = pressed ? 90.0f : axisToAngle(xRaw);

  // Exponential smoothing so the horn sweeps instead of snapping.
  angle += (target - angle) * SMOOTHING;
  servoWriteAngle(angle);

  if (++tick >= 10) {                          // ~5 prints per second
    tick = 0;
    Serial.printf("X=%4d  Y=%4d  SEL=%d  ->  angle=%5.1f deg\n",
                  xRaw, yRaw, pressed ? 1 : 0, angle);
  }

  delay(LOOP_PERIOD_MS);
}
