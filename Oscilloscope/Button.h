#pragma once
#include <Arduino.h>

// Interrupt-driven push button (active LOW, internal pull-up).
// Presses are never missed even while a capture is running.
class Button {
public:
  void begin(uint8_t pin);
  bool consumePress();   // true once per press
};
