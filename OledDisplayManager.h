// OledDisplayManager.h
#pragma once

// ThingPulse OLED SSD1306 - Ref: https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <SSD1306Wire.h>

class OledDisplayManager {
public:
  OledDisplayManager();
  void begin();
  void showSplashScreen();
  void updateDisplay(const char* presetName, int currentChordIndex, int numChords, const int* activeNotes, int noteCount, int octave, int transpose, bool isRootOnlyMode);

private:
  SSD1306Wire display;

  void drawKey(int x, int y, int radius, bool isActive, bool isRoot);
  void drawKeyboard(const int* activeNotes, int noteCount);
};
