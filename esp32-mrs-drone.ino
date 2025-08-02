
// Arduino BLE-MIDI - Ref: https://github.com/lathoub/Arduino-BLE-MIDI
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// ThingPulse OLED SSD1306 - Ref: https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <Wire.h>
#include "SSD1306Wire.h"

#include <JC_Button.h> // Ref: https://github.com/JChristensen/JC_Button

// Struct for preset.
struct ChordPreset {
  const char* name;
  const int (*chords)[4];
  int length;
};

// Preset 1 : IM7 > IVM7
const int preset1[][4] = {
  {60, 64, 67, 71}, // CM7
  {65, 69, 72, 76}  // FM7
};

// Preset 2 : IIm7 > V7 > IM7
const int preset2[][4] = {
  {62, 65, 67, 72}, // Dm7
  {67, 71, 74, 77}, // V7(G7)
  {60, 64, 67, 71} // CM7
};

// Preset list.
ChordPreset presets[] = {
  {"IM7 > IVM7", preset1, 2},
  {"IIm7 > V7 > IM7", preset2, 3}
};

// Pin Definitions.
// Safe GPIO pins for switch/button input on ESP32:
// 4, 5, 13, 14, 16, 17, 25, 26, 27, 32, 33
const int PIN_FOOTSWITCH = 5;
const int PIN_PRESET_BUTTON = 4;

// Buttons and switches.
Button footSwitch(PIN_FOOTSWITCH, 50);
Button presetButton(PIN_PRESET_BUTTON, 50);

// OLED.
SSD1306Wire display(0x3C, SDA, SCL);

// MIDI.
const int MIDI_CH = 1;
BLEMIDI_CREATE_INSTANCE("BLE MIDI", MIDI);

// States.
int currentPreset = 0;
int currentChordIndex = 0;

void setup(){
  // Buttons and switches.
  footSwitch.begin();
  presetButton.begin();

  // MIDI
  BLEMIDI.setHandleConnected(OnConnected);
  BLEMIDI.setHandleDisconnected(OnDisconnected);
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

void loop(){
  // Foot switch.
  footSwitch.read();
  if (footSwitch.wasPressed()) {
    Serial.println("Footswitch: pressed");
    ChordPreset& preset = presets[currentPreset];
    const int* chord = preset.chords[currentChordIndex];

    for (int i = 0; i < 4; i++) {
      int note = chord[i];
      MIDI.sendNoteOn(note, 127, MIDI_CH);
    }

    // TODO: drawStatusScreen();
  }

  if(footSwitch.wasReleased()){
    Serial.println("Footswitch: Released");
    ChordPreset& preset = presets[currentPreset];
    const int* chord = preset.chords[currentChordIndex];
    // Send Note off
    for (int i = 0; i < 4; i++) {
      int note = chord[i];
      MIDI.sendNoteOff(note, 0, MIDI_CH);
    }
    // Step to next chord.
    currentChordIndex = (currentChordIndex + 1) % preset.length;

    // TODO: drawStatusScreen();
  }

  // Change presets.
  presetButton.read();
  if (presetButton.wasPressed()) {
    currentPreset = (currentPreset + 1) % (sizeof(presets) / sizeof(presets[0]));
    currentChordIndex = 0;
    drawStatusScreen();
  }
}

void drawStatusScreen() {
  display.clear();

  // Display current preset.
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, presets[currentPreset].name);
  
  // Display "MONO" "POLY" mode.
  display.setFont(ArialMT_Plain_10);
  String modeText = isMono ? "MONO" : "POLY";
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 54, modeText);

  display.display();
}

void OnConnected() {
  // TODO: Light up blue LED.
}

void OnDisconnected() {
  // TODO: Unlight blue LED.
}
   