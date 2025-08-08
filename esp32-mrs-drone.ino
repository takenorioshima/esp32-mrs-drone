// Arduino BLE-MIDI - Ref: https://github.com/lathoub/Arduino-BLE-MIDI
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

#include <JC_Button.h>      // Ref: https://github.com/JChristensen/JC_Button
#include <RotaryEncoder.h>  // Ref: https://github.com/mathertel/RotaryEncoder

#include "FootSwitchManager.h"
#include "OledDisplayManager.h"
#include "ChordPresets.h"

// Pin Definitions.
// Safe GPIO pins for switch/button input on ESP32:
// 2, 4, 5, 13, 14, 15, 16, 17, 18, 19, 21, 23, 24, 25, 26, 27, 32, 33
const int PIN_FOOTSWITCH = 5;
const int PIN_PRESET_BUTTON = 4;
const int PIN_ENCODER_S1 = 19;
const int PIN_ENCODER_S2 = 18;
const int PIN_ENCODER_BUTTON = 15;

// Buttons, switches and encoder.
FootSwitchManager footSwitch(PIN_FOOTSWITCH);
Button presetButton(PIN_PRESET_BUTTON, 50);
Button encoderButton(PIN_ENCODER_BUTTON, 50);

RotaryEncoder encoder(PIN_ENCODER_S1, PIN_ENCODER_S2, RotaryEncoder::LatchMode::TWO03);
int encoderLastPos = encoder.getPosition();
bool isEncoderButtonLongPressed = false;
unsigned long encoderButtonLastPressedAt = 0;
const unsigned long ENCODER_BUTTON_LONG_PRESS_THRESHOLD = 200;

// OLED.
OledDisplayManager oled;

// MIDI.
const int MIDI_CH = 1;
BLEMIDI_CREATE_INSTANCE("BLE MIDI", MIDI);

int currentPreset = 0;
int currentChordIndex = 0;
int transpose = 0;
int activeNotes[NOTES_PER_CHORD];
int activeNoteCount = 0;
bool isRootOnlyMode = false;
bool isPresetChanged = false;

bool stateChanged = false;

void drawStatusScreen() {
  oled.updateDisplay(presets[currentPreset].name, currentChordIndex, presets[currentPreset].numChords, activeNotes, activeNoteCount, transpose, isRootOnlyMode);
}

void sendChordNoteOn(const int* chord) {
  if (isRootOnlyMode) {
    activeNoteCount = 1;
    int note = chord[0] + transpose;
    MIDI.sendNoteOn(note, 127, MIDI_CH);
    activeNotes[0] = note;
    stateChanged = true;
    return;
  }

  activeNoteCount = 0;
  for (int i = 0; i < NOTES_PER_CHORD; i++) {
    int note = chord[i];
    if (note == -1) continue;
    note += transpose;
    MIDI.sendNoteOn(note, 127, MIDI_CH);
    activeNotes[activeNoteCount++] = note;
  }
  stateChanged = true;
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
  stateChanged = true;
}

void handleBLEMIDIConnected() {
  // TODO: Light up blue LED.
}

void handleBLEMIDIOnDisonnected() {
  // TODO: Unlight blue LED.
}

void handleHoldModeOn() {
  Serial.println("MODE: HOLD ON");
}

void handleTapOnHold() {
  Serial.println("MODE: HOLD TAP");
  sendChordNoteOff();

  const ChordPreset& preset = presets[currentPreset];
  if (!isPresetChanged) {
    // Step to next chord.
    currentChordIndex = (currentChordIndex + 1) % preset.numChords;
  } else {
    isPresetChanged = false;
  }
  const int* chord = preset.chords[currentChordIndex];

  sendChordNoteOn(chord);
}

void handleMomentaryOn() {
  Serial.println("MODE: MOMENTARY ON");

  const ChordPreset& preset = presets[currentPreset];
  const int* chord = preset.chords[currentChordIndex];
  sendChordNoteOn(chord);
}

void handleMomentaryOff() {
  Serial.println("MODE: MOMENTARY OFF");
  sendChordNoteOff();

  // Step to next chord.
  const ChordPreset& preset = presets[currentPreset];
  currentChordIndex = (currentChordIndex + 1) % preset.numChords;
}

void setup() {
  Serial.begin(9600);

  // Buttons, switches and encoder.
  presetButton.begin();
  encoderButton.begin();
  encoder.setPosition(0);

  footSwitch.begin();
  footSwitch.onEnterHoldCallback(handleHoldModeOn);
  footSwitch.onHoldTapCallback(handleTapOnHold);
  footSwitch.onExitHoldCallback(handleMomentaryOff);
  footSwitch.onMomentaryStartCallback(handleMomentaryOn);
  footSwitch.onMomentaryEndCallback(handleMomentaryOff);

  // MIDI
  BLEMIDI.setHandleConnected(handleBLEMIDIConnected);
  BLEMIDI.setHandleDisconnected(handleBLEMIDIOnDisonnected);
  MIDI.begin();

  // OLED
  oled.begin();
  oled.showSplashScreen();
  drawStatusScreen();
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
    stateChanged = true;
  }

  encoderButton.read();
  if (encoderButton.wasPressed()) {
    unsigned long now = millis();
    encoderButtonLastPressedAt = now;
    Serial.println("Encoder Button: Pressed");
  }
  if (encoderButton.wasReleased()) {
    if (isEncoderButtonLongPressed) {
      isEncoderButtonLongPressed = false;
      return;
    }
    Serial.println("Encoder Button: Released(Short Press)");
    // Reset transpose.
    transpose = 0;
    stateChanged = true;
  }
  if (!isEncoderButtonLongPressed) {
    if (encoderButton.isPressed() && (millis() - encoderButtonLastPressedAt > ENCODER_BUTTON_LONG_PRESS_THRESHOLD)) {
      Serial.println("Encoder Button: Released(Long Press)");
      // Toggle Root-Only Mode
      isRootOnlyMode = !isRootOnlyMode;
      isEncoderButtonLongPressed = true;
      stateChanged = true;
    }
  }

  // Foot switch.
  footSwitch.update();

  // Change presets.
  presetButton.read();
  if (presetButton.wasPressed()) {
    currentPreset = (currentPreset + 1) % (sizeof(presets) / sizeof(presets[0]));
    currentChordIndex = 0;
    stateChanged = true;

    // Ignore code index increment when changing preset in hold mode
    if (footSwitch.getMode() == MODE_HOLD) {
      isPresetChanged = true;
    }
  }

  // Update display
  if (stateChanged) {
    drawStatusScreen();
    stateChanged = false;
  }
}
