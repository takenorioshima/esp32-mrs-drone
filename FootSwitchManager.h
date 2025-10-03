#pragma once
#include <JC_Button.h>

enum FootSwitchMode {
  MODE_NONE,
  MODE_MOMENTARY,
  MODE_HOLD
};

class FootSwitchManager {
public:
  FootSwitchManager(int pin, unsigned long longPressMs = 400, unsigned long doubleTapMs = 300);

  void begin();
  void update();

  FootSwitchMode getMode() const;

  // Set callbacks
  void onMomentaryStartCallback(void (*cb)());
  void onMomentaryEndCallback(void (*cb)());
  void onEnterHoldCallback(void (*cb)());
  void onExitHoldCallback(void (*cb)());
  void onHoldTapCallback(void (*cb)());

private:
  Button button;
  FootSwitchMode mode = MODE_NONE;
  unsigned long lastPressedAt = 0;
  const unsigned long LONG_PRESS_THRESHOLD;
  const unsigned long DOUBLE_TAP_THRESHOLD;
  
  // Callback placeholders
  void (*onMomentaryStart)() = nullptr;
  void (*onMomentaryEnd)() = nullptr;
  void (*onEnterHold)() = nullptr;
  void (*onExitHold)() = nullptr;
  void (*onHoldTap)() = nullptr;
};
