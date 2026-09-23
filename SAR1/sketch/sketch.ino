// SAR 1 - ESP32 Push-Button RGB LED Color Cycler
// ENGR 4399 ST: Cyber-Physical & IoT Systems | Daniel Critchlow Jr. | Fall 2026
//
// One momentary push button steps a common-cathode RGB LED through a
// four-state cycle: Off -> Red -> Green -> Blue -> Off.
//
// Wiring (see diagram.json):
//   Push button -> GPIO12, other side to GND (INPUT_PULLUP, active LOW)
//   RGB Red     -> GPIO25 via series resistor
//   RGB Green   -> GPIO26 via series resistor
//   RGB Blue    -> GPIO27 via series resistor
//   RGB COM     -> GND (common cathode)

const int BUTTON_PIN = 12; // Button connected to GPIO 12
const int RED_PIN    = 25; // Red LED connected to GPIO 25
const int GREEN_PIN  = 26; // Green LED connected to GPIO 26
const int BLUE_PIN   = 27; // Blue LED connected to GPIO 27

int colorState = 0;
int lastButtonState = HIGH;

// Debounce variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Start with all LEDs off
  setColor(0, 0, 0);
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  // Check if the button state changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only act if the state has settled longer than the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button is pressed (LOW due to INPUT_PULLUP)
    if (reading == LOW && lastButtonState == HIGH) {
      colorState = (colorState + 1) % 4; // Cycle through 4 states (Off, Red, Green, Blue)

      switch (colorState) {
        case 0: setColor(0, 0, 0);   break; // Off
        case 1: setColor(1, 0, 0);   break; // Red
        case 2: setColor(0, 1, 0);   break; // Green
        case 3: setColor(0, 0, 1);   break; // Blue
      }
    }
  }

  lastButtonState = reading;
}

// Helper function to set digital RGB states (1 = On, 0 = Off)
void setColor(int red, int green, int blue) {
  digitalWrite(RED_PIN, red);
  digitalWrite(GREEN_PIN, green);
  digitalWrite(BLUE_PIN, blue);
}
