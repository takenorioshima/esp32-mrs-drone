// Arduino BLE-MIDI - Ref: https://github.com/lathoub/Arduino-BLE-MIDI
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// ThingPulse OLED SSD1306 - Ref: https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <Wire.h>
#include "SSD1306Wire.h"

#include <JC_Button.h>  // Ref: https://github.com/JChristensen/JC_Button

#include <RotaryEncoder.h>  // Ref: https://github.com/mathertel/RotaryEncoder

#define NOTES_PER_CHORD 4

// Preset settings.
struct ChordPreset {
  const char* name;
  const int (*chords)[NOTES_PER_CHORD];
  int length;
};

#define CHORD_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

// Preset 1
const int chords1[][NOTES_PER_CHORD] = {
  { 60, 64, 67, 71 },  // CM7
  { 53, 57, 60, 64 }   // FM7
};
ChordPreset preset1 = { "IM7 > IVM7", chords1, CHORD_LENGTH(chords1) };

// Preset 2
const int chords2[][NOTES_PER_CHORD] = {
  { 62, 64, 69, -1 },
  { 55, 62, 69, -1 },
  { 60, 67, 71, -1 }
};
ChordPreset preset2 = { "IIm7 > V7 > IM7", chords2, CHORD_LENGTH(chords2) };

// Preset 3
const int chords3[][NOTES_PER_CHORD] = {
  { 60, 67, 74, -1 },
  { 58, 65, 72, -1 }
};
ChordPreset preset3 = { "Isus2 > bVIIsus2", chords3, CHORD_LENGTH(chords3) };

// Preset list
ChordPreset presets[] = {
  preset1,
  preset2,
  preset3
};

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

// OLED.
SSD1306Wire display(0x3C, SDA, SCL);

const char* noteNames[12] = {
  "C", "Db", "D", "Eb", "E", "F",
  "Gb", "G", "Ab", "A", "Bb", "B"
};

// MIDI.
const int MIDI_CH = 1;
BLEMIDI_CREATE_INSTANCE("BLE MIDI", MIDI);

// States.
int currentPreset = 0;
int currentChordIndex = 0;
int transpose = 0;
int activeNotes[NOTES_PER_CHORD];
int activeNoteCount = 0;

bool isFootSwitchPressed = false;
bool isFootSwitchHold = false;
unsigned long footSwitchPressedAt = 0;

void drawKeyboard() {
  // Debug output
  for (int i = 0; i < NOTES_PER_CHORD; i++) {
    Serial.print("activeNotes[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(activeNotes[i]);
  }

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
    isFootSwitchPressed = true;
    footSwitchPressedAt = millis();
    Serial.println("Footswitch: Pressed");
    ChordPreset& preset = presets[currentPreset];
    const int* chord = preset.chords[currentChordIndex];

    activeNoteCount = 0;

    for (int i = 0; i < NOTES_PER_CHORD; i++) {
      int note = chord[i];
      if (note == -1) continue;
      note += transpose;
      MIDI.sendNoteOn(note, 127, MIDI_CH);
      activeNotes[activeNoteCount++] = note;
    }

    drawStatusScreen();
  }

  if (!isFootSwitchHold && isFootSwitchPressed && (millis() - footSwitchPressedAt > LONG_PRESS_THRESHOLD)) {
    isFootSwitchHold = true;
    Serial.println("Footswitch: Hold");
  }

  if (footSwitch.wasReleased()) {
    if (isFootSwitchHold) {
      Serial.println("Footswitch: Hold Released");
      isFootSwitchHold = false;
      isFootSwitchPressed = false;
    } else {
      Serial.println("Footswitch: Released");
      isFootSwitchPressed = false;
    }

    for (int i = 0; i < activeNoteCount; i++) {
      MIDI.sendNoteOff(activeNotes[i], 0, MIDI_CH);
    }

    // Reset activeNotes to clear keyboard display
    for (int i = 0; i < NOTES_PER_CHORD; i++) {
      activeNotes[i] = -1;
    }
    activeNoteCount = 0;

    // Step to next chord.
    ChordPreset& preset = presets[currentPreset];
    currentChordIndex = (currentChordIndex + 1) % preset.length;

    // Optionally update display here
    drawStatusScreen();
  }

  // Change presets.
  presetButton.read();
  if (presetButton.wasPressed()) {
    currentPreset = (currentPreset + 1) % (sizeof(presets) / sizeof(presets[0]));
    currentChordIndex = 0;
    drawStatusScreen();
  }
}
