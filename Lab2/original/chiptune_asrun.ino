/*
  Chiptune Jukebox  (ESP32)
  ---------------------------------------------------------------
  Songs: Pokemon Battle ->  Attack on Titan  ->  Fullmetal Alchemist  ->  Super Mario

  Wiring:
    - Passive buzzer / piezo:  pin 27  ->  buzzer  ->  GND
    - Push button:             pin 4   ->  button  ->  GND
      (uses the internal pull-up, so no resistor needed)

  How it works:
    - Starts on the Pokemon battle theme and plays through.
    - Press the button any time to skip to the next song.
    - If a song finishes on its own, it waits silently until
      you press the button to start the next one.
    - Songs cycle: Battle -> Attack on Titan -> Fullmetal Alchemist -> Mario -> Battle ...

  Note format:
    Each melody is a list of { note, duration } pairs.
    duration: 1 = whole, 2 = half, 4 = quarter, 8 = eighth, 16 = sixteenth.
    A negative duration means a dotted note (1.5x length).
  ---------------------------------------------------------------
*/

int buzzer    = 27;
int buttonPin = 26;

// --- Note frequencies in Hz (shared by all songs) ---
#define REST 0
#define E4   330
#define FS4  370
#define G4   392
#define GS4  415
#define A4   440
#define AS4  466
#define B4   494
#define C5   523
#define CS5  554
#define D5   587
#define DS5  622
#define E5   659
#define F5   698
#define FS5  740
#define G5   784
#define GS5  831
#define A5   880
#define AS5  932
#define B5   988
#define C6   1047
#define CS6  1109
#define D6   1175
#define DS6  1245
#define E6   1319
#define F6   1397
#define G6   1568

// =====================================================================
//  SONG 1 : Pokemon Red/Blue - Trainer Battle Theme
// =====================================================================
int pokemonBattle[] = {
  A5,16, GS5,16, G5,16, FS5,16, A5,16, F5,16, FS5,16, F5,16,
  A5,16, E5,16, F5,16, E5,16, A5,16, DS5,16, E5,16, DS5,16,
  A5,16, D5,16, DS5,16, D5,16, A5,16, CS5,16, D5,16, CS5,16,
  A5,16, C5,16, CS5,16, C5,16, A5,16, B4,16, C5,16, B4,16,
  B5,8, REST,8, REST,4, REST,2, REST,1, B5,8, REST,8, REST,4,
  REST,2, REST,2, REST,4, A5,8, REST,8, B4,8, REST,4, CS5,8,
  REST,4, D5,8, REST,8, B4,8, CS5,8, REST,8, D5,8, REST,4,
  A5,8, AS5,8, B5,8, REST,4, CS6,8, REST,4, D6,8, REST,8,
  B5,8, CS6,8, REST,8, D6,8, REST,4, A5,8, REST,8, B4,4,
  B4,8, FS4,2, REST,8, REST,4, B4,4, FS4,4, B4,4, C5,1,
  REST,1, B4,4, B4,8, FS4,2, REST,8, REST,4, B4,4, FS4,4,
  B4,4, A4,1, REST,1, G4,1, D5,2, G4,2, A4,1, REST,1,
  G4,1, E5,2, FS5,2, E5,1, G5,4, A5,8, G5,8, FS5,8,
  E5,8, D5,8, E5,8, FS5,1, FS5,1, G5,1, G5,4, A5,8,
  G5,8, G5,8, FS5,8, E5,8, FS5,8, GS5,1, GS5,1, A5,1,
  CS6,2, E6,2, D6,4, A5,4, C6,8, B5,8, B5,1, B5,4,
  B5,2, D6,4, A5,4, AS5,8, F6,8, F6,2, F6,4, G6,1,
  E6,1, E6,2, E6,4, E6,8, B4,8, C5,4, C5,8, A4,8,
  A4,2, REST,4, C5,4, A4,4, C5,4, AS4,4, AS4,8, F5,2,
  REST,4, REST,8, AS4,4, F5,4, D5,4, C5,4, C5,8, A4,8,
  A4,2, REST,4, A4,4, E5,8, D5,8, C5,8, E5,8, D5,8,
  AS4,4, F5,8, F5,2, G5,4, G5,8, F5,4, F5,8, D5,4,
  F5,1, E5,1, D5,1, E5,1, F6,1, E6,1, G6,1, F6,1,
  E5,8, REST,4, E5,8, REST,4, E5,8, REST,8, E5,8, REST,4,
  E5,8, REST,4, E5,8, REST,8, E5,8, REST,4, E5,8, REST,4,
  E5,8, REST,8, E5,8, REST,4, E5,8, REST,4, E5,8, REST,8,
  A4,4, B4,4, G4,8, A4,8, A4,2, B4,8, CS5,8, E5,8,
  D5,8, CS5,8, B4,8, AS4,1, REST,8, AS4,8, C5,8, F5,8,
  E5,8, D5,8, C5,8, AS4,8, B4,1, REST,8, B4,8, CS5,8,
  G5,8, FS5,8, E5,8, D5,8, B4,8, C5,1, E5,2, G5,2,
  B4,4, B4,8, FS4,2, REST,8, REST,4, B4,4, FS4,4, B4,4,
  C5,1, REST,1, B4,4, B4,8, FS4,2, REST,8, REST,4, B4,4,
  FS4,4, B4,4, A4,1, REST,1, G4,1, D5,2, G4,2, A4,1,
  REST,1, G4,1, E5,2, FS5,2, E5,1, G5,4, A5,8, G5,8,
  FS5,8, E5,8, D5,8, E5,8, FS5,1, FS5,1, G5,1, G5,4,
  A5,8, G5,8, G5,8, FS5,8, E5,8, FS5,8, GS5,1, GS5,1,
  A5,1, CS6,2, E6,2, D6,4, A5,4, C6,8, B5,8, B5,1,
  B5,4, B5,2, D6,4, A5,4, AS5,8, F6,8, F6,2, F6,4,
  G6,1, E6,1, E6,2, E6,4, E6,8, B4,8, C5,4, C5,8,
  A4,8, A4,2, REST,4, C5,4, A4,4, C5,4, AS4,4, AS4,8,
  F5,2, REST,4, REST,8, AS4,4, F5,4, D5,4, C5,4, C5,8,
  A4,8, A4,2, REST,4, A4,4, E5,8, D5,8, C5,8, E5,8,
  D5,8, AS4,4, F5,8, F5,2, G5,4, G5,8, F5,4, F5,8,
  D5,4, F5,1, E5,1, D5,1, E5,1, F6,1, E6,1, G6,1,
  F6,1, E5,8, REST,4, E5,8, REST,4, E5,8, REST,8, E5,8,
  REST,4, E5,8, REST,4, E5,8, REST,8, E5,8, REST,4, E5,8,
  REST,4, E5,8, REST,8, E5,8, REST,4, E5,8, REST,4, E5,8,
  REST,8, A4,4, B4,4, G4,8, A4,8, A4,2, B4,8, CS5,8,
  E5,8, D5,8, CS5,8, B4,8, AS4,1, REST,8, AS4,8, C5,8,
  F5,8, E5,8, D5,8, C5,8, AS4,8, B4,1, REST,8, B4,8,
  CS5,8, G5,8, FS5,8, E5,8, D5,8, B4,8, C5,1, E5,2,
  G5,2, D5,1
};

// =====================================================================
//  SONG 2 : Attack on Titan - Guren no Yumiya (intro)
// =====================================================================
int attackOnTitan[] = {
  D5,8, D5,8, F5,8, E5,4, C5,8, C5,8, D5,4, D5,8,
  F5,8, E5,4, C5,4, REST,8, A5,4, F5,8, G5,8, REST,8,
  E5,8, REST,8, F5,8, REST,8, D5,8, REST,8, E5,8, REST,8,
  C5,4, REST,8, A5,4, F5,8, G5,8, REST,8, E5,8, REST,8,
  F5,8, REST,8, E5,8, REST,8, D5,8, REST,8, C5,4, REST,8,
  C6,4, GS5,8, AS5,8, REST,8, G5,8, REST,8, GS5,8, REST,8,
  F5,8, REST,8, G5,8, REST,8, DS5,4, REST,8, C6,4, GS5,8,
  AS5,8, REST,8, G5,8, REST,8, GS5,8, REST,8, G5,8, REST,8,
  F5,8, REST,8, DS5,4, REST,8
};


// =====================================================================
//  SONG 3 : Fullmetal Alchemist - Melissa (intro)
// =====================================================================
int fullmetalAlchemist[] = {
  E5,8, B5,4, E5,8, E5,4, D5,4, E5,8, B5,8, E5,8,
  E5,8, D5,16, E5,16, E5,8, FS5,4, FS5,16, FS5,16, FS5,8,
  G5,8, FS5,8, G5,8, FS5,8, B4,8, B4,4, REST,1
};

// =====================================================================
//  SONG 4 : Super Mario Bros - Overworld Theme (lead melody)
// =====================================================================
int superMario[] = {
  E5,8, E5,8, REST,8, E5,8, REST,8, C5,8, E5,8, REST,8,
  G5,8, REST,8, REST,4, G4,8, REST,4, REST,8, C5,8, REST,4,
  G4,8, REST,4, E4,8, REST,4, A4,8, REST,8, B4,8, REST,8,
  AS4,8, A4,8, REST,8, G4,8, E5,8, REST,8, G5,8, A5,8,
  REST,8, F5,8, G5,8, REST,8, E5,8, REST,8, C5,8, D5,8,
  B4,8, REST,4, C5,8, REST,4, G4,8, REST,4, E4,8, REST,4,
  A4,8, REST,8, B4,8, REST,8, AS4,8, A4,8, REST,8, G4,8,
  E5,8, REST,8, G5,8, A5,8, REST,8, F5,8, G5,8, REST,8,
  E5,8, REST,8, C5,8, D5,8, B4,8, REST,4, REST,4, G5,8,
  FS5,8, F5,8, DS5,8, REST,8, E5,8, REST,8, GS4,8, A4,8,
  C5,8, C5,8, A4,8, C5,8, D5,8, REST,4, G5,8, FS5,8,
  F5,8, DS5,8, REST,8, E5,8, REST,8, C6,8, REST,8, C6,8,
  C6,8, REST,8, REST,2, G5,8, FS5,8, F5,8, DS5,8, REST,8,
  E5,8, REST,8, GS4,8, A4,8, C5,8, C5,8, A4,8, C5,8,
  D5,8, REST,4, DS5,8, REST,4, D5,8, REST,4, C5,8, REST,4,
  REST,8, REST,2, REST,4, G5,8, FS5,8, F5,8, DS5,8, REST,8,
  E5,8, REST,8, GS4,8, A4,8, C5,8, C5,8, A4,8, C5,8,
  D5,8, REST,4, G5,8, FS5,8, F5,8, DS5,8, REST,8, E5,8,
  REST,8, C6,8, REST,8, C6,8, C6,8, REST,8, REST,2, G5,8,
  FS5,8, F5,8, DS5,8, REST,8, E5,8, REST,8, GS4,8, A4,8,
  C5,8, C5,8, A4,8, C5,8, D5,8, REST,4, DS5,8, REST,4,
  D5,8, REST,4, C5,8, REST,4, REST,8, REST,2, C5,8, C5,8,
  REST,8, C5,8, REST,8, C5,8, D5,8, REST,8, E5,8, C5,8,
  REST,8, A4,8, G4,8, REST,8, REST,4, C5,8, C5,8, REST,8,
  C5,8, REST,8, C5,8, D5,8, E5,8, REST,2, REST,2, C5,8,
  C5,8, REST,8, C5,8, REST,8, C5,8, D5,8, REST,8, E5,8,
  C5,8, REST,8, A4,8, G4,8, REST,8, REST,4, E5,8, E5,8,
  REST,8, E5,8, REST,8, C5,8, E5,8, REST,8, G5,8, REST,8,
  REST,4, G4,8, REST,8, REST,4, C5,8, REST,4, G4,8, REST,4,
  E4,8, REST,4, A4,8, REST,8, B4,8, REST,8, AS4,8, A4,8,
  REST,8, G4,8, E5,8, REST,8, G5,8, A5,8, REST,8, F5,8,
  G5,8, REST,8, E5,8, REST,8, C5,8, D5,8, B4,8, REST,4,
  C5,8, REST,4, G4,8, REST,4, E4,8, REST,4, A4,8, REST,8,
  B4,8, REST,8, AS4,8, A4,8, REST,8, G4,8, E5,8, REST,8,
  G5,8, A5,8, REST,8, F5,8, G5,8, REST,8, E5,8, REST,8,
  C5,8, D5,8, B4,8, REST,4, E5,8, C5,8, REST,8, G4,8,
  REST,4, GS4,8, REST,8, A4,8, F5,8, REST,8, F5,8, A4,8,
  REST,8, REST,4, B4,8, A5,8, REST,8, A5,8, A5,8, G5,8,
  REST,8, F5,8, E5,8, C5,8, REST,8, A4,8, G4,8, REST,8,
  REST,4, E5,8, C5,8, REST,8, G4,8, REST,4, GS4,8, REST,8,
  A4,8, F5,8, REST,8, F5,8, A4,8, REST,8, REST,4, B4,8,
  F5,8, REST,8, F5,8, F5,8, E5,8, REST,8, D5,8, C5,8,
  REST,8, REST,4, REST,2, E5,8, C5,8, REST,8, G4,8, REST,4,
  GS4,8, REST,8, A4,8, F5,8, REST,8, F5,8, A4,8, REST,8,
  REST,4, B4,8, A5,8, REST,8, A5,8, A5,8, G5,8, REST,8,
  F5,8, E5,8, C5,8, REST,8, A4,8, G4,8, REST,8, REST,4,
  E5,8, C5,8, REST,8, G4,8, REST,4, GS4,8, REST,8, A4,8,
  F5,8, REST,8, F5,8, A4,8, REST,8, REST,4, B4,8, F5,8,
  REST,8, F5,8, F5,8, E5,8, REST,8, D5,8, C5,8, REST,8,
  REST,4, REST,2, C5,8, C5,8, REST,8, C5,8, REST,8, C5,8,
  D5,8, REST,8, E5,8, C5,8, REST,8, A4,8, G4,8, REST,8,
  REST,4, C5,8, C5,8, REST,8, C5,8, REST,8, C5,8, D5,8,
  E5,8, REST,2, REST,2, C5,8, C5,8, REST,8, C5,8, REST,8,
  C5,8, D5,8, REST,8, E5,8, C5,8, REST,8, A4,8, G4,8,
  REST,8, REST,4, E5,8, E5,8, REST,8, E5,8, REST,8, C5,8,
  E5,8, REST,8, G5,8, REST,8, REST,4, G4,8, REST,8, REST,4,
  E5,8, C5,8, REST,8, G4,8, REST,4, GS4,8, REST,8, A4,8,
  F5,8, REST,8, F5,8, A4,8, REST,8, REST,4, B4,8, A5,8,
  REST,8, A5,8, A5,8, G5,8, REST,8, F5,8, E5,8, C5,8,
  REST,8, A4,8, G4,8, REST,8, REST,4, E5,8, C5,8, REST,8,
  G4,8, REST,4, GS4,8, REST,8, A4,8, F5,8, REST,8, F5,8,
  A4,8, REST,8, REST,4, B4,8, F5,8, REST,8, F5,8, F5,8,
  E5,8, REST,8, D5,8, C5,8, REST,8, REST,4, REST,2, C5,8,
  REST,4, G4,8, REST,4, E4,8, REST,8, A4,8, B4,8, A4,8,
  GS4,4, AS4,4, GS4,4, G4,1
};

// =====================================================================
//  Song table  (add/reorder songs here; count is automatic)
// =====================================================================
struct Song {
  int *data;
  int numNotes;   // number of note/duration pairs
  int tempo;      // BPM
};

Song songs[] = {
  { pokemonBattle, sizeof(pokemonBattle) / sizeof(int) / 2, 180 },
  { attackOnTitan, sizeof(attackOnTitan) / sizeof(int) / 2, 150 },
  { fullmetalAlchemist, sizeof(fullmetalAlchemist) / sizeof(int) / 2, 135 },
  { superMario,        sizeof(superMario)        / sizeof(int) / 2, 200 }
};
int numSongs    = sizeof(songs) / sizeof(songs[0]);
int currentSong = 0;

// =====================================================================
//  Button (debounced, edge-detected)
// =====================================================================
bool buttonPressed() {
  static bool last = HIGH;
  static unsigned long t = 0;
  bool now = digitalRead(buttonPin);
  if (now != last && millis() - t > 40) {
    t = millis();
    last = now;
    if (now == LOW) return true;   // pressed (pin pulled to GND)
  }
  return false;
}

// Delay for ms, but bail out early if the button is pressed.
bool delayOrButton(int ms) {
  unsigned long start = millis();
  while (millis() - start < (unsigned long)ms) {
    if (buttonPressed()) return true;
    delay(2);
  }
  return false;
}

// Play one song. Returns true if the button interrupted it.
bool playSong(int index) {
  int *m        = songs[index].data;
  int count     = songs[index].numNotes;
  int wholenote = (60000 * 4) / songs[index].tempo;

  for (int i = 0; i < count * 2; i += 2) {
    int note    = m[i];
    int divider = m[i + 1];
    int duration;

    if (divider > 0) duration = wholenote / divider;
    else             duration = (wholenote / abs(divider)) * 1.5;   // dotted note

    if (note == REST) noTone(buzzer);
    else              tone(buzzer, note, duration * 0.9);

    if (delayOrButton(duration)) {   // interrupted mid-song
      noTone(buzzer);
      return true;
    }
    noTone(buzzer);
  }
  return false;   // finished naturally
}

// =====================================================================
//  Main
// =====================================================================
void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  bool interrupted = playSong(currentSong);

  if (!interrupted) {
    // Song ended on its own — wait here until a press.
    while (!buttonPressed()) {
      delay(5);
    }
  }

  currentSong = (currentSong + 1) % numSongs;   // advance to next song
  delay(150);                                    // brief gap before it starts
}