#include "FootSwitchManager.h"

FootSwitchManager::FootSwitchManager(int pin, unsigned long longPressMs, unsigned long doubleTapMs)
  : button(pin),
    LONG_PRESS_THRESHOLD(longPressMs),
    DOUBLE_TAP_THRESHOLD(doubleTapMs) {}

void FootSwitchManager::begin() {
  button.begin();
}

void FootSwitchManager::update() {
  button.read();

  if (button.wasPressed()) {
    unsigned long now = millis();

    if (mode == MODE_HOLD) {
      if (onHoldTap) onHoldTap();
      if (now - lastPressedAt < DOUBLE_TAP_THRESHOLD) {
        mode = MODE_NONE;
        if (onExitHold) onExitHold();
        return;
      }
    } else {
      mode = MODE_MOMENTARY;
      if (onMomentaryStart) onMomentaryStart();
    }

    lastPressedAt = now;
  }

  if (button.wasReleased()) {
    if (mode == MODE_NONE) {
      return;
    }

    unsigned long pressDuration = millis() - lastPressedAt;

    if (pressDuration < LONG_PRESS_THRESHOLD) {
      if (mode == MODE_MOMENTARY) {
        if (onEnterHold) onEnterHold();
        mode = MODE_HOLD;
      }
    } else {
      if (mode == MODE_MOMENTARY) {
        if (onMomentaryEnd) onMomentaryEnd();
        mode = MODE_NONE;
      }
      if (mode == MODE_HOLD) {
        if (onExitHold) onExitHold();
        mode = MODE_NONE;
      }
    }
  }
}

FootSwitchMode FootSwitchManager::getMode() const {
  return mode;
}

// Callback setters
void FootSwitchManager::onMomentaryStartCallback(void (*cb)()) {
  onMomentaryStart = cb;
}

void FootSwitchManager::onMomentaryEndCallback(void (*cb)()) {
  onMomentaryEnd = cb;
}

void FootSwitchManager::onEnterHoldCallback(void (*cb)()) {
  onEnterHold = cb;
}

void FootSwitchManager::onExitHoldCallback(void (*cb)()) {
  onExitHold = cb;
}

void FootSwitchManager::onHoldTapCallback(void (*cb)()) {
  onHoldTap = cb;
}
