// Arduino BLE-MIDI - Ref: https://github.com/lathoub/Arduino-BLE-MIDI
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

#include <JC_Button.h>      // Ref: https://github.com/JChristensen/JC_Button
#include <RotaryEncoder.h>  // Ref: https://github.com/mathertel/RotaryEncoder

#include "OledDisplayManager.h"
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
OledDisplayManager oled;

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

void drawStatusScreen() {
  oled.updateDisplay(presets[currentPreset].name, activeNotes, activeNoteCount, transpose);
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
  drawStatusScreen();
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
  drawStatusScreen();
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
  oled.begin();
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

        return;
      }
    } else {
      // Start momentary mode
      playMode = MOMENTARY;
      Serial.println("MODE: MOMENTARY ON");

      ChordPreset& preset = presets[currentPreset];
      const int* chord = preset.chords[currentChordIndex];
      sendChordNoteOn(chord);
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

      if (playMode == HOLD) {
        // Tap on hold mode
        Serial.println("MODE: HOLD TAP");
        sendChordNoteOff();

        // Step to next chord.
        ChordPreset& preset = presets[currentPreset];
        currentChordIndex = (currentChordIndex + 1) % preset.length;
        const int* chord = preset.chords[currentChordIndex];

        sendChordNoteOn(chord);
      } else {
        // First tap to enter hold mode
        Serial.println("MODE: HOLD ON");
        ChordPreset& preset = presets[currentPreset];
        const int* chord = preset.chords[currentChordIndex];
        sendChordNoteOn(chord);
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
