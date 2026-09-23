/************************************************************
  Lab 2 - ESP32 Three-Voice Chiptune Jukebox with OLED
  ENGR 4399 ST: Cyber-Physical & IoT Systems
  Daniel Critchlow Jr. - Fall 2026
  ------------------------------------------------------------
  Three passive buzzers play lead, harmony and bass at the same
  time while an SSD1306 OLED shows what is playing, how far in it
  is, which voices are sounding, and a sprite that dances on the
  beat. Three buttons are the whole interface.

  This is the multi-voice successor to the single-buzzer jukebox in
  original/chiptune_asrun.ino. The interesting change is not the
  extra buzzers, it is that nothing blocks any more: the old sketch
  played a note with tone() and then sat in delay() for its
  duration, which cannot service three voices or redraw a display.
  See chiptune.h for how the sequencer replaces that.

  ------------------------------------------------------------
  WIRING

  Buzzers (passive piezo; one leg to the GPIO, the other to GND):
      LEAD    -> GPIO 27
      HARMONY -> GPIO 26
      BASS    -> GPIO 25

  OLED SSD1306 128x64, I2C, address 0x3C:
      SDA -> GPIO 21
      SCL -> GPIO 22
      VCC -> 3V3          GND -> GND

  Buttons (other leg to GND; internal pull-ups, no resistors):
      NEXT  -> GPIO 32    next track
      PLAY  -> GPIO 33    play / pause
      MIX   -> GPIO 4     cycle ALL -> LEAD -> LEAD+BASS

  Libraries (Library Manager):
      - Adafruit GFX
      - Adafruit SSD1306
  Board: ESP32 Dev Module, arduino-esp32 core 3.x

  Note on the core version: this sketch uses ledcAttach() and
  ledcWriteTone(), which are the 3.x API. On 2.x those are
  ledcSetup() + ledcAttachPin(), and ledcWrite() takes a channel
  number instead of a pin.
************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"
#include "notes.h"
#include "chiptune.h"
#include "sprite.h"
#include "songs_original.h"
#include "songs_classic.h"
#include "songs_user.h"

// Set to 0 to drop the four single-voice melodies carried over from
// the original sketch and build only the three-voice tracks.
#define ENABLE_CLASSIC_SONGS 1

// When a track ends, move to the next one by itself. Set to 0 to
// make the player wait for a button press, like the original did.
#define AUTO_ADVANCE 1
#define GAP_MS 400

// =====================================================================
//  Playlist
// =====================================================================
// Voice order is always { lead, harmony, bass }. NO_SCORE leaves a
// voice silent, which is how the single-voice melodies are carried
// over unchanged - they play on the lead buzzer alone.
const Song PLAYLIST[] = {
  { "Rival Encounter",  "Battle Theme",  168,
    { SCORE(rivalLead),   SCORE(rivalHarm),   SCORE(rivalBass)   } },
  { "Crimson Vanguard", "Anime Opening", 152,
    { SCORE(crimsonLead), SCORE(crimsonHarm), SCORE(crimsonBass) } },
  { "Neon Alchemy",     "Slow Theme",    120,
    { SCORE(neonLead),    SCORE(neonHarm),    SCORE(neonBass)    } },
  { "Trap Arcade",      "Trap / 8-bit",  142,
    { SCORE(trapLead),    SCORE(trapHarm),    SCORE(trapBass)    } },

#if ENABLE_CLASSIC_SONGS
  { "Pokemon Battle",   "Lead only",     180,
    { SCORE(pokemonBattle),      NO_SCORE, NO_SCORE } },
  { "Attack on Titan",  "Lead only",     150,
    { SCORE(attackOnTitan),      NO_SCORE, NO_SCORE } },
  { "Fullmetal Alch.",  "Lead only",     135,
    { SCORE(fullmetalAlchemist), NO_SCORE, NO_SCORE } },
  { "Super Mario",      "Lead only",     200,
    { SCORE(superMario),         NO_SCORE, NO_SCORE } },
#endif

#if HAVE_USER_SONG
  { USER_SONG_TITLE,    USER_SONG_STYLE, USER_SONG_BPM,
    { SCORE(userLead),    SCORE(userHarm),    SCORE(userBass)    } },
#endif
};
const uint8_t NUM_SONGS = sizeof(PLAYLIST) / sizeof(PLAYLIST[0]);

// =====================================================================
//  Hardware
// =====================================================================
Adafruit_SSD1306 display(OLED_W, OLED_H, &Wire, -1);
ChiptunePlayer   player;

const uint8_t VOICE_PINS[NUM_VOICES] = { PIN_LEAD, PIN_HARM, PIN_BASS };
const uint8_t VOICE_DUTY[NUM_VOICES] = { DUTY_LEAD, DUTY_HARM, DUTY_BASS };
const char   *VOICE_TAG[NUM_VOICES]  = { "L", "H", "B" };

uint8_t  currentSong  = 0;
uint32_t lastDisplay  = 0;
uint32_t resumeAt     = 0;      // millis() at which the next track starts

// =====================================================================
//  Debounced, active-LOW button  (same pattern as Lab 1)
// =====================================================================
struct Button {
  uint8_t  pin;
  bool     stableState;
  bool     lastReading;
  uint32_t lastChange;
};

void buttonInit(Button &b, uint8_t pin) {
  b.pin         = pin;
  b.stableState = HIGH;
  b.lastReading = HIGH;
  b.lastChange  = 0;
  pinMode(pin, INPUT_PULLUP);
}

// True exactly once per press, on the HIGH -> LOW edge.
bool buttonPressed(Button &b) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastReading) {
    b.lastChange  = millis();
    b.lastReading = reading;
  }
  if ((millis() - b.lastChange) > DEBOUNCE_MS && reading != b.stableState) {
    b.stableState = reading;
    if (b.stableState == LOW) return true;
  }
  return false;
}

Button btnNext, btnPlay, btnMix;

// =====================================================================
//  Display
// =====================================================================
#define METER_X      10
#define METER_W      80
#define METER_H       6
#define METER_Y0     22
#define METER_STEP    9
#define SPRITE_X    102
#define SPRITE_Y     22
#define BAR_Y        48
#define BAR_H         7

// Map a frequency onto a bar length. Pitch is logarithmic, so the
// bar is too - otherwise the bass voice never leaves the left edge
// and the lead pins to the right.
int meterLength(int freq) {
  if (freq <= 0) return 0;
  float t = (logf((float)freq) - logf(90.0f)) /
            (logf(2100.0f) - logf(90.0f));
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;
  // returns a width for the INNER fill, which is METER_W - 2 wide
  return (int)(t * (METER_W - 4)) + 2;      // always show at least a stub
}

void printTime(uint32_t ms) {
  uint32_t s = ms / 1000;
  display.printf("%u:%02u", s / 60, s % 60);
}

void renderNowPlaying(uint32_t now) {
  const Song *s = player.song();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // --- title ---
  display.setCursor(0, 0);
  display.print(s->title);

  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  // --- tempo, track number, mix ---
  display.setCursor(0, 12);
  display.printf("%3u BPM", (unsigned)s->bpm);
  display.setCursor(48, 12);
  display.printf("%u/%u", (unsigned)(currentSong + 1), (unsigned)NUM_SONGS);
  display.setCursor(80, 12);
  display.printf("%-6s", MIX_NAMES[player.mix()]);

  // --- one meter per voice ---
  for (uint8_t v = 0; v < NUM_VOICES; v++) {
    int y = METER_Y0 + v * METER_STEP;
    display.setCursor(0, y);
    display.print(VOICE_TAG[v]);

    display.drawRect(METER_X, y, METER_W, METER_H, SSD1306_WHITE);

    if (player.voiceMuted(v)) {
      // Struck through rather than blank, so a muted voice still reads
      // as a voice and not as one that happens to be resting.
      display.drawLine(METER_X + 2, y + METER_H / 2,
                       METER_X + METER_W - 3, y + METER_H / 2,
                       SSD1306_WHITE);
      continue;
    }
    int len = meterLength(player.voiceNote(v));
    if (len > 0)
      display.fillRect(METER_X + 1, y + 1, len, METER_H - 2, SSD1306_WHITE);
  }

  // --- the dancer ---
  uint8_t frame = player.playing()
                    ? (uint8_t)(player.beatIndex(now) % SPRITE_FRAMES)
                    : 0;
  display.drawBitmap(SPRITE_X, SPRITE_Y, danceFrames[frame],
                     SPRITE_W, SPRITE_H, SSD1306_WHITE);

  // --- progress ---
  uint32_t el  = player.elapsedMs(now);
  uint32_t tot = player.totalMs();
  display.drawRect(0, BAR_Y, 128, BAR_H, SSD1306_WHITE);
  if (tot > 0) {
    int w = (int)((uint64_t)el * 126ULL / tot);
    if (w > 126) w = 126;
    if (w > 0) display.fillRect(1, BAR_Y + 1, w, BAR_H - 2, SSD1306_WHITE);
  }

  display.setCursor(0, 57);
  printTime(el);
  display.print(F(" / "));
  printTime(tot);

  display.setCursor(78, 57);
  if (!player.playing() && !player.finished()) display.print(F("PAUSED"));
  else if (player.passes() > 1)
    display.printf("x%u/%u", (unsigned)(player.passIndex() + 1),
                   (unsigned)player.passes());

  display.display();
}

// The OLED is the one thing in this sketch that blocks. A full
// 128x64 frame is 1024 bytes, and at 400 kHz that is about 23 ms
// inside display(). If that lands on top of a note onset the note
// starts late, so the renderer asks the sequencer how much time it
// has before the next onset and skips the frame when the answer is
// "not enough". DISPLAY_FORCE_MS keeps the UI from freezing if the
// music never offers a gap - at that point a little jitter is the
// better trade.
void serviceDisplay(uint32_t now) {
  uint32_t since = now - lastDisplay;
  if (since < DISPLAY_MIN_MS) return;
  if (since < DISPLAY_FORCE_MS && player.slackUs(now) < DISPLAY_SLACK_US)
    return;
  lastDisplay = now;
  renderNowPlaying(now);
}

// =====================================================================
//  Track changes
// =====================================================================
void announce() {
  const Song *s = player.song();
  Serial.printf("[play] %u/%u  %-18s %-14s %u BPM  %lu ms  x%u\n",
                (unsigned)(currentSong + 1), (unsigned)NUM_SONGS,
                s->title, s->style, (unsigned)s->bpm,
                (unsigned long)player.totalMs(), (unsigned)player.passes());
}

void startSong(uint8_t index) {
  currentSong = index % NUM_SONGS;
  player.start(&PLAYLIST[currentSong], millis());
  announce();
  lastDisplay = 0;                 // redraw immediately
}

void nextSong() { startSong(currentSong + 1); }

// =====================================================================
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(50);

  buttonInit(btnNext, PIN_BTN_NEXT);
  buttonInit(btnPlay, PIN_BTN_PLAY);
  buttonInit(btnMix,  PIN_BTN_MIX);

  player.begin(VOICE_PINS, VOICE_DUTY);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(I2C_HZ);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed - halting"));
    for (;;) delay(1000);
  }

  // Splash
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("CHIPTUNE JUKEBOX"));
  display.drawLine(0, 9, 127, 9, SSD1306_WHITE);
  display.setCursor(0, 14);
  display.println(F("3 voices / ESP32"));
  display.printf("%u tracks\n", (unsigned)NUM_SONGS);
  display.println(F("NEXT PLAY MIX"));
  display.drawBitmap(SPRITE_X, SPRITE_Y + 8, danceFrames[2],
                     SPRITE_W, SPRITE_H, SSD1306_WHITE);
  display.display();

  Serial.printf("\nLab 2 - three-voice chiptune jukebox, %u tracks\n",
                (unsigned)NUM_SONGS);
  player.audit(PLAYLIST, NUM_SONGS);     // warn about voice-length mismatches

  delay(1500);
  startSong(0);
}

// =====================================================================
void loop() {
  uint32_t now = millis();

  // --- input ---
  if (buttonPressed(btnNext)) {
    resumeAt = 0;
    nextSong();
  }

  if (buttonPressed(btnPlay)) {
    if (player.finished()) startSong(currentSong);   // replay
    else                   player.togglePause(now);
    Serial.printf("[transport] %s\n",
                  player.playing() ? "playing" : "paused");
    lastDisplay = 0;
  }

  if (buttonPressed(btnMix)) {
    player.cycleMix();
    Serial.printf("[mix] %s\n", MIX_NAMES[player.mix()]);
    lastDisplay = 0;
  }

  // --- audio: the only thing that must never be late ---
  player.update(now);

  // --- end of track ---
  if (player.finished()) {
#if AUTO_ADVANCE
    if (resumeAt == 0) resumeAt = now + GAP_MS;
    else if ((int32_t)(now - resumeAt) >= 0) {
      resumeAt = 0;
      nextSong();
    }
#endif
  }

  // --- display: best effort, gives way to the music ---
  serviceDisplay(now);
}
