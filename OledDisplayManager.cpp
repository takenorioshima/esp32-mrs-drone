// OledDisplayManager.cpp
#include "OledDisplayManager.h"

OledDisplayManager::OledDisplayManager()
  : display(0x3C, SDA, SCL) {}

void OledDisplayManager::begin() {
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
  display.display();
}

void OledDisplayManager::updateDisplay(const char* presetName, int currentChordIndex, int numChords, const int* activeNotes, int noteCount, int transpose) {
  display.clear();

  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, presetName);

  // Display keyboard
  drawKeyboard(activeNotes, noteCount);

  // Display code index
  String codeIndexText = String(currentChordIndex + 1) + " / " + String(numChords);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 54, codeIndexText);

  // Display transpose
  String transposeText = (transpose > 0 ? "+" : "") + String(transpose);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 54, transposeText);

  display.display();
}

void OledDisplayManager::drawKey(int x, int y, int radius, bool isActive, bool isRoot) {
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

void OledDisplayManager::drawKeyboard(const int* activeNotes, int noteCount) {
  const int baseX = 16;
  const int baseY = 40;
  const int radius = 8;
  const int whiteKeySpacing = 16;

  int rootKey = -1;
  for (int i = 0; i < noteCount; i++) {
    if (activeNotes[i] != -1) {
      rootKey = activeNotes[i] % 12;
      break;
    }
  }

  const int whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
  for (int i = 0; i < 7; i++) {
    int noteKey = whiteNotes[i];
    int x = baseX + i * whiteKeySpacing;

    bool isActive = false;
    bool isRoot = false;

    for (int j = 0; j < noteCount; j++) {
      if (activeNotes[j] == -1) continue;
      if ((activeNotes[j] % 12) == noteKey) {
        isActive = true;
        if (noteKey == rootKey) isRoot = true;
      }
    }

    drawKey(x, baseY, radius, isActive, isRoot);
  }

  const int blackNotes[] = { 1, 3, 6, 8, 10 };
  const int blackKeyXOffsets[] = { 1, 2, 4, 5, 6 };

  for (int i = 0; i < 5; i++) {
    int noteKey = blackNotes[i];
    int x = baseX + blackKeyXOffsets[i] * whiteKeySpacing - 8;
    int y = baseY - 14;

    bool isActive = false;
    bool isRoot = false;

    for (int j = 0; j < noteCount; j++) {
      if (activeNotes[j] == -1) continue;
      if ((activeNotes[j] % 12) == noteKey) {
        isActive = true;
        if (noteKey == rootKey) isRoot = true;
      }
    }

    drawKey(x, y, radius, isActive, isRoot);
  }
}
