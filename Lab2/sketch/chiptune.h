/************************************************************
  chiptune.h - the three-voice sequencer
  Lab 2: ESP32 Three-Voice Chiptune Jukebox
  ------------------------------------------------------------
  WHY THIS IS NOT tone() + delay()

  The single-buzzer version of this project played a note with
  tone() and then blocked in delay() for its duration. That works
  for one voice and cannot work for three: on arduino-esp32,
  tone() owns one LEDC channel and retargets it at whatever pin
  you last named, so a second tone() call silences the first. And
  a blocking delay() cannot service two other voices that need to
  change notes partway through.

  This sequencer fixes both problems:

    * Each voice owns its own LEDC channel, attached once in
      begin() and never re-pointed, so all three sound together.

    * Nothing blocks. update() is called from loop() as often as
      possible and does work only when a note is actually due.

  ------------------------------------------------------------
  ABSOLUTE SCHEDULING (the part worth understanding)

  The naive non-blocking sequencer keeps a per-voice countdown and
  reloads it when it hits zero. That accumulates error: each note
  is rounded to whole milliseconds, and each rounding is inherited
  by every note after it. The lead voice, made of many short notes,
  rounds far more often than the bass voice, made of few long ones,
  so the two drift apart - slowly, audibly, and worse the longer
  the track runs.

  Instead, every note onset here is stored as an ABSOLUTE position
  in microseconds from the start of the track:

      onset[n+1] = onset[n] + duration[n]

  computed in 64-bit microseconds and compared against the song
  clock. A late update() - because an I2C display write blocked
  for 20 ms, say - delays that one onset but not the next, because
  the next one's position was never relative to it. The error is
  bounded jitter instead of unbounded drift.

  Voice 0 (lead) is authoritative for length. Voices 1 and 2 wrap
  back to their own start if they are shorter, which lets a
  one-bar accompaniment loop under a sixteen-bar melody, and are
  truncated if they are longer. All three are realigned at every
  pass boundary, so nothing can creep between repeats.
  begin() reports any mismatch over Serial.
************************************************************/

#ifndef CHIPTUNE_H
#define CHIPTUNE_H

#include <Arduino.h>
#include "config.h"
#include "notes.h"

#define NUM_VOICES 3
#define VOICE_LEAD 0
#define VOICE_HARM 1
#define VOICE_BASS 2

// ---------------------------------------------------------------
//  Score data
// ---------------------------------------------------------------
// A score is the flat {note, divider} int array used throughout
// this project. SCORE() fills in the pair count so it can never
// disagree with the array.
struct Score {
  const int *data;
  uint16_t   pairs;
};

#define SCORE(arr) { (arr), (uint16_t)(sizeof(arr) / sizeof(int) / 2) }
#define NO_SCORE   { nullptr, 0 }

struct Song {
  const char *title;    // <= 21 chars renders on one OLED line
  const char *style;
  uint16_t    bpm;
  Score       voice[NUM_VOICES];   // lead, harmony, bass
};

// ---------------------------------------------------------------
//  Voice mix - what the three buzzers are allowed to play
// ---------------------------------------------------------------
enum MixMode { MIX_ALL = 0, MIX_LEAD, MIX_LEAD_BASS, MIX_COUNT };
static const char *const MIX_NAMES[MIX_COUNT] = { "ALL", "LEAD", "L+BASS" };

// ---------------------------------------------------------------
//  Duration of one note, in microseconds.
//    divider > 0 : 1/divider of a whole note
//    divider < 0 : dotted, 1.5x of 1/|divider|
// ---------------------------------------------------------------
static inline uint64_t noteDurationUs(int divider, uint64_t wholeUs) {
  if (divider == 0) return 0;
  if (divider > 0)  return wholeUs / (uint32_t)divider;
  return (wholeUs * 3ULL) / (2ULL * (uint32_t)(-divider));
}

static inline uint64_t scoreDurationUs(const Score &s, uint64_t wholeUs) {
  uint64_t t = 0;
  for (uint16_t i = 0; i < s.pairs; i++)
    t += noteDurationUs(s.data[2 * i + 1], wholeUs);
  return t;
}

// Raise a frequency by a whole number of octaves (see BASS_OCTAVE_SHIFT).
static inline int shiftOctaves(int freq, int semitones) {
  if (freq == REST || semitones == 0) return freq;
  while (semitones >= 12) { freq *= 2; semitones -= 12; }
  while (semitones <= -12) { freq /= 2; semitones += 12; }
  return freq;
}

// ---------------------------------------------------------------
//  Player
// ---------------------------------------------------------------
class ChiptunePlayer {
 public:
  void begin(const uint8_t pins[NUM_VOICES], const uint8_t dutyPct[NUM_VOICES]) {
    for (uint8_t v = 0; v < NUM_VOICES; v++) {
      _pin[v]  = pins[v];
      _duty[v] = dutyPct[v];
      // One LEDC channel per pin, attached once and never re-pointed.
      // 1000 Hz is just a placeholder carrier; every note overwrites it.
      ledcAttach(_pin[v], 1000, LEDC_BITS);
      silence(v);
    }
  }

  // ---- transport ----------------------------------------------
  void start(const Song *song, uint32_t nowMs) {
    _song  = song;
    // A whole note is four beats: 4 * 60 s / bpm, in microseconds.
    _wholeUs = 240000000ULL / (uint32_t)song->bpm;
    _beatUs  = _wholeUs / 4;

    _passUs = scoreDurationUs(song->voice[VOICE_LEAD], _wholeUs);
    if (_passUs == 0) _passUs = _wholeUs;      // empty lead: play one bar of silence

    // Repeat whichever whole number of times lands closest to the
    // target. round(target/pass) == (2*target + pass) / (2*pass).
    uint64_t target = (uint64_t)TARGET_PLAY_MS * 1000ULL;
    _passes = (uint16_t)((2 * target + _passUs) / (2 * _passUs));
    if (_passes < 1) _passes = 1;
    _totalUs = _passUs * _passes;

    _startMs   = nowMs;
    _holdUs    = 0;
    _passIndex = 0;
    _playing   = true;
    _done      = false;
    resetVoices(0);
  }

  void pause(uint32_t nowMs) {
    if (!_playing || _done) return;
    _holdUs  = elapsedUs(nowMs);
    _playing = false;
    for (uint8_t v = 0; v < NUM_VOICES; v++) silence(v);
  }

  void resume(uint32_t nowMs) {
    if (_playing || _done) return;
    _startMs = nowMs - (uint32_t)(_holdUs / 1000ULL);
    _playing = true;
    // Voices pick up from wherever the score says they should be.
    resetVoices(elapsedUs(nowMs));
  }

  void togglePause(uint32_t nowMs) { _playing ? pause(nowMs) : resume(nowMs); }

  void stop() {
    _playing = false;
    _done    = true;
    for (uint8_t v = 0; v < NUM_VOICES; v++) silence(v);
  }

  // ---- the work ------------------------------------------------
  void update(uint32_t nowMs) {
    if (!_playing || _done || _song == nullptr) return;

    const uint64_t el = elapsedUs(nowMs);

    if (el >= _totalUs) { stop(); return; }

    // Realign all three voices whenever a repeat begins. This is
    // what guarantees that nothing creeps from pass to pass.
    uint16_t pass = (uint16_t)(el / _passUs);
    if (pass != _passIndex) {
      _passIndex = pass;
      resetVoices((uint64_t)pass * _passUs);
    }

    for (uint8_t v = 0; v < NUM_VOICES; v++) service(v, el);
  }

  // ---- what the UI needs --------------------------------------
  bool     playing()   const { return _playing; }
  bool     finished()  const { return _done; }
  const Song *song()   const { return _song; }
  uint8_t  mix()       const { return _mix; }
  void     setMix(uint8_t m) {
    _mix = m % MIX_COUNT;
    for (uint8_t v = 0; v < NUM_VOICES; v++)
      if (!audible(v)) silence(v);
  }
  void cycleMix() { setMix(_mix + 1); }

  // Frequency each voice is sounding right now, 0 when silent.
  int  voiceNote(uint8_t v) const { return _vs[v].sounding ? _vs[v].note : 0; }
  bool voiceMuted(uint8_t v) const { return !audible(v); }

  uint32_t elapsedMs(uint32_t nowMs) const {
    uint64_t e = _playing ? elapsedUs(nowMs) : _holdUs;
    if (e > _totalUs) e = _totalUs;
    return (uint32_t)(e / 1000ULL);
  }
  uint32_t totalMs()  const { return (uint32_t)(_totalUs / 1000ULL); }
  uint16_t passes()   const { return _passes; }
  uint16_t passIndex() const { return _passIndex; }

  // Which beat of the track we are on - the sprite dances on this.
  uint32_t beatIndex(uint32_t nowMs) const {
    if (_beatUs == 0) return 0;
    uint64_t e = _playing ? elapsedUs(nowMs) : _holdUs;
    return (uint32_t)(e / _beatUs);
  }

  // Microseconds until the next note onset on any voice. The display
  // uses this to decide whether it has time to push a frame.
  uint64_t slackUs(uint32_t nowMs) const {
    if (!_playing || _done) return 0xFFFFFFFFULL;
    uint64_t el = elapsedUs(nowMs);
    uint64_t best = 0xFFFFFFFFULL;
    for (uint8_t v = 0; v < NUM_VOICES; v++) {
      if (_vs[v].idle) continue;
      uint64_t n = _vs[v].nextUs;
      if (n <= el) return 0;
      if (n - el < best) best = n - el;
    }
    return best;
  }

  // Report voice-length mismatches once, at startup.
  void audit(const Song *songs, uint8_t count) const {
    for (uint8_t s = 0; s < count; s++) {
      uint64_t w = 240000000ULL / songs[s].bpm;
      uint64_t lead = scoreDurationUs(songs[s].voice[VOICE_LEAD], w);
      if (lead == 0) continue;
      for (uint8_t v = 1; v < NUM_VOICES; v++) {
        uint64_t d = scoreDurationUs(songs[s].voice[v], w);
        if (d == 0 || d == lead) continue;
        Serial.printf("[audit] \"%s\" voice %u is %lu ms vs lead %lu ms - "
                      "it will %s\n",
                      songs[s].title, v,
                      (unsigned long)(d / 1000), (unsigned long)(lead / 1000),
                      d < lead ? "loop" : "be truncated");
      }
    }
  }

 private:
  struct VoiceState {
    uint16_t idx;        // next pair to load
    uint64_t nextUs;     // absolute onset of that pair
    uint64_t offUs;      // absolute release of the note now sounding
    int      note;       // frequency of the note now sounding
    bool     sounding;
    bool     idle;       // this voice has no score
  };

  const Song *_song    = nullptr;
  uint8_t     _pin[NUM_VOICES]  = {0, 0, 0};
  uint8_t     _duty[NUM_VOICES] = {50, 50, 50};
  VoiceState  _vs[NUM_VOICES]   = {};
  uint64_t    _wholeUs = 0, _beatUs = 0, _passUs = 0, _totalUs = 0;
  uint64_t    _holdUs  = 0;          // elapsed time frozen by pause()
  uint32_t    _startMs = 0;
  uint16_t    _passes = 1, _passIndex = 0;
  bool        _playing = false, _done = true;
  uint8_t     _mix = MIX_ALL;

  uint64_t elapsedUs(uint32_t nowMs) const {
    return (uint64_t)(nowMs - _startMs) * 1000ULL;
  }

  bool audible(uint8_t v) const {
    switch (_mix) {
      case MIX_LEAD:      return v == VOICE_LEAD;
      case MIX_LEAD_BASS: return v == VOICE_LEAD || v == VOICE_BASS;
      default:            return true;
    }
  }

  void resetVoices(uint64_t atUs) {
    for (uint8_t v = 0; v < NUM_VOICES; v++) {
      _vs[v].idx      = 0;
      _vs[v].nextUs   = atUs;
      _vs[v].offUs    = atUs;
      _vs[v].note     = REST;
      _vs[v].sounding = false;
      _vs[v].idle     = (_song == nullptr) ||
                        (_song->voice[v].pairs == 0) ||
                        (_song->voice[v].data == nullptr);
      silence(v);
    }
  }

  void service(uint8_t v, uint64_t el) {
    VoiceState &st = _vs[v];
    if (st.idle) return;
    const Score &sc = _song->voice[v];

    // Release the current note a little early, so repeated notes
    // are heard separately.
    if (st.sounding && el >= st.offUs) {
      silence(v);
      st.sounding = false;
    }

    // Load every note whose onset has already arrived. Normally
    // this runs at most once; it runs more only when update() was
    // called late, and then it catches up rather than drifting.
    uint8_t guard = 0;
    while (el >= st.nextUs && guard++ < 64) {
      if (st.idx >= sc.pairs) st.idx = 0;        // shorter voice: loop it

      int note    = sc.data[2 * st.idx];
      int divider = sc.data[2 * st.idx + 1];
      st.idx++;

      uint64_t d = noteDurationUs(divider, _wholeUs);
      if (d == 0) d = 1000;                      // malformed entry: 1 ms

      uint64_t onset = st.nextUs;
      st.nextUs = onset + d;

      uint64_t gate = d / GATE_DIVISOR;
      if (gate < GATE_MIN_US) gate = GATE_MIN_US;
      if (gate > GATE_MAX_US) gate = GATE_MAX_US;
      if (gate >= d) gate = d / 2;               // very short note
      st.offUs = st.nextUs - gate;

      st.note = note;
      // Skip notes we have already run past (catch-up), and notes
      // on a voice the mix has muted.
      if (note != REST && el < st.offUs && audible(v)) {
        int f = note;
        if (v == VOICE_BASS) f = shiftOctaves(f, BASS_OCTAVE_SHIFT);
        sound(v, f);
        st.sounding = true;
      } else {
        silence(v);
        st.sounding = false;
      }
    }
  }

  void sound(uint8_t v, int freq) {
    // ledcWriteTone() sets the carrier and a 50 % duty; the
    // ledcWrite() that follows replaces the duty with this voice's,
    // which is what gives each voice its own loudness and timbre.
    ledcWriteTone(_pin[v], (uint32_t)freq);
    ledcWrite(_pin[v], ((uint32_t)LEDC_MAX * _duty[v]) / 100);
  }

  void silence(uint8_t v) { ledcWrite(_pin[v], 0); }
};

#endif  // CHIPTUNE_H
