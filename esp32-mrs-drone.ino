// Arduino BLE-MIDI - Ref: https://github.com/lathoub/Arduino-BLE-MIDI
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// ThingPulse OLED SSD1306 - Ref: https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <Wire.h>
#include "SSD1306Wire.h"

#include <JC_Button.h>  // Ref: https://github.com/JChristensen/JC_Button
#include <RotaryEncoder.h>  // Ref: https://github.com/mathertel/RotaryEncoder

#include "ChordPresets.h"

// Pin Definitions.
// Safe GPIO pins for switch/button input on ESP32:
// 4, 5, 13, 14, 16, 17, 25, 26, 27, 32, 33
const int PIN_FOOTSWITCH = 5;
const int PIN_PRESET_BUTTON = 4;
const int PIN_ENCODER_S1 = 19;
const int PIN_ENCODER_S2 = 18;

// Buttons, switches and encoder.
Button footSwitch(PIN_FOOTSWITCH, 50);
Button presetButton(PIN_PRESET_BUTTON, 50);

RotaryEncoder encoder(PIN_ENCODER_S1, PIN_ENCODER_S2, RotaryEncoder::LatchMode::TWO03);
int encoderLastPos = encoder.getPosition();

const unsigned long LONG_PRESS_THRESHOLD = 600;
const unsigned long DOUBLE_TAP_THRESHOLD = 300;

// OLED.
SSD1306Wire display(0x3C, SDA, SCL);

// MIDI.
const int MIDI_CH = 1;
BLEMIDI_CREATE_INSTANCE("BLE MIDI", MIDI);

// States.
enum PlayMode {
  NONE,
  MOMENTARY,
  HOLD
};
PlayMode playMode = HOLD;

int currentPreset = 0;
int currentChordIndex = 0;
int transpose = 0;
int activeNotes[NOTES_PER_CHORD];
int activeNoteCount = 0;

bool isSkipNextRelease = false;
unsigned long footSwitchPressedAt = 0;
unsigned long lastFootSwitchPressedTime = 0;

unsigned long lastPressTime = 0;
const unsigned long DOUBLE_CLICK_THRESHOLD = 400;  // ms

void drawKeyboard() {
  // Debug output
  // for (int i = 0; i < NOTES_PER_CHORD; i++) {
  //   Serial.print("activeNotes[");
  //   Serial.print(i);
  //   Serial.print("] = ");
  //   Serial.println(activeNotes[i]);
  // }

  const int baseX = 16;
  const int baseY = 40;
  const int radius = 8;
  const int whiteKeySpacing = 16;

  // Set Root key
  int rootKey = -1;
  for (int i = 0; i < NOTES_PER_CHORD; i++) {
    if (activeNotes[i] != -1) {
      rootKey = activeNotes[i] % 12;
      break;
    }
  }

  // White keys: C D E F G A B
  const int whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
  for (int i = 0; i < 7; i++) {
    int noteKey = whiteNotes[i];
    int x = baseX + i * whiteKeySpacing;

    bool isActive = false;
    bool isRoot = false;

    for (int j = 0; j < NOTES_PER_CHORD; j++) {
      if (activeNotes[j] == -1) continue;
      if ((activeNotes[j] % 12) == noteKey) {
        isActive = true;
        if (noteKey == rootKey) isRoot = true;
      }
    }

    drawKey(x, baseY, radius, isActive, isRoot);
  }

  // Black keys: Db, Eb, Gb, Ab, Bb
  const int blackNotes[] = { 1, 3, 6, 8, 10 };
  const int blackKeyXOffsets[] = { 1, 2, 4, 5, 6 };

  for (int i = 0; i < 5; i++) {
    int noteKey = blackNotes[i];
    int x = baseX + blackKeyXOffsets[i] * whiteKeySpacing - 8;
    int y = baseY - 14;

    bool isActive = false;
    bool isRoot = false;

    for (int j = 0; j < NOTES_PER_CHORD; j++) {
      if (activeNotes[j] == -1) continue;
      if ((activeNotes[j] % 12) == noteKey) {
        isActive = true;
        if (noteKey == rootKey) isRoot = true;
      }
    }

    drawKey(x, y, radius, isActive, isRoot);
  }
}

// Draw individual key.
void drawKey(int x, int y, int radius, bool isActive, bool isRoot) {
  if (isActive) {
    display.fillCircle(x, y, radius);
    if (isRoot) {
      display.setColor(BLACK);
      display.fillCircle(x, y, radius / 2);
      display.setColor(WHITE);
    }
  } else {
    display.drawCircle(x, y, radius);
  }
}

void drawStatusScreen() {
  display.clear();

  // Display current preset name
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, presets[currentPreset].name);

  // Draw keyboard GUI
  drawKeyboard();

  // Display current transpose
  String transposeText = (transpose > 0 ? "+" : "") + String(transpose);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 54, transposeText);

  display.display();
}

void sendChordNoteOn(const int* chord) {
  activeNoteCount = 0;
  for (int i = 0; i < NOTES_PER_CHORD; i++) {
    int note = chord[i];
    if (note == -1) continue;
    note += transpose;
    MIDI.sendNoteOn(note, 127, MIDI_CH);
    activeNotes[activeNoteCount++] = note;
  }
}

void sendChordNoteOff() {
  for (int i = 0; i < activeNoteCount; i++) {
    MIDI.sendNoteOff(activeNotes[i], 0, MIDI_CH);
  }
  // Reset
  for (int i = 0; i < NOTES_PER_CHORD; i++) {
    activeNotes[i] = -1;
  }
  activeNoteCount = 0;
}

void handleBLEMIDIConnected() {
  // TODO: Light up blue LED.
}

void handleBLEMIDIOnDisonnected() {
  // TODO: Unlight blue LED.
}

void setup() {
  // Buttons, switches and encoder.
  footSwitch.begin();
  presetButton.begin();
  encoder.setPosition(0);

  // MIDI
  BLEMIDI.setHandleConnected(handleBLEMIDIConnected);
  BLEMIDI.setHandleDisconnected(handleBLEMIDIOnDisonnected);
  MIDI.begin();

  // OLED
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  drawStatusScreen();

  Serial.begin(9600);
}

void loop() {
  // Encoder.
  encoder.tick();
  int encoderNewPos = encoder.getPosition() * 0.5;
  if (encoderNewPos != encoderLastPos) {
    int delta = encoderNewPos - encoderLastPos;
    transpose += delta;
    transpose = constrain(transpose, -12, 12);
    encoderLastPos = encoderNewPos;

    Serial.print("Transpose: ");
    Serial.println(transpose);
    drawStatusScreen();
  }

  // Foot switch.
  footSwitch.read();
  if (footSwitch.wasPressed()) {
    unsigned long now = millis();

    if (playMode == HOLD) {
      if (now - footSwitchPressedAt < DOUBLE_TAP_THRESHOLD) {
        // Double tap: exit hold mode
        isSkipNextRelease = true;
        playMode = NONE;
        Serial.println("MODE: HOLD OFF");
        sendChordNoteOff();

        // Step to next chord.
        ChordPreset& preset = presets[currentPreset];
        currentChordIndex = (currentChordIndex + 1) % preset.length;

        drawStatusScreen();
        return;
      }
    } else {
      // Start momentary mode
      playMode = MOMENTARY;
      Serial.println("MODE: MOMENTARY ON");

      ChordPreset& preset = presets[currentPreset];
      const int* chord = preset.chords[currentChordIndex];
      sendChordNoteOn(chord);

      drawStatusScreen();
    }

    footSwitchPressedAt = now;
  }

  if (footSwitch.wasReleased()) {
    if (isSkipNextRelease) {
      // Ignore this release due to prior double tap
      isSkipNextRelease = false;
      Serial.println("RELEASE SKIPPED");
      return;
    }

    if (millis() - footSwitchPressedAt < LONG_PRESS_THRESHOLD) {

      // Short press: cancel momentary mode on
      if (playMode == MOMENTARY) {
        Serial.println("MODE: MOMENTARY CANCEL");
        sendChordNoteOff();
      }

      if(playMode == HOLD){
        // Tap on hold mode
        Serial.println("MODE: HOLD TAP");
        sendChordNoteOff();

        // Step to next chord.
        ChordPreset& preset = presets[currentPreset];
        currentChordIndex = (currentChordIndex + 1) % preset.length;
        const int* chord = preset.chords[currentChordIndex];
        
        sendChordNoteOn(chord);

        drawStatusScreen();
      } else {
        // First tap to enter hold mode
        Serial.println("MODE: HOLD ON");
        ChordPreset& preset = presets[currentPreset];
        const int* chord = preset.chords[currentChordIndex];
        sendChordNoteOn(chord);
        
        drawStatusScreen();
      }

      playMode = HOLD;

    } else {
      if (playMode == MOMENTARY) {
        // Long press release
        Serial.println("MODE: MOMENTARY OFF");
        sendChordNoteOff();
        playMode = NONE;

        // Step to next chord.
        ChordPreset& preset = presets[currentPreset];
        currentChordIndex = (currentChordIndex + 1) % preset.length;

        drawStatusScreen();
      }
    }
  }

  // Change presets.
  presetButton.read();
  if (presetButton.wasPressed()) {
    currentPreset = (currentPreset + 1) % (sizeof(presets) / sizeof(presets[0]));
    currentChordIndex = 0;
    drawStatusScreen();
  }
}
