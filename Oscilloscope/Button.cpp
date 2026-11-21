#include "Button.h"
#include "Config.h"
#include "esp_timer.h"

static volatile bool     s_pressed = false;
static volatile uint32_t s_lastMs  = 0;

static void IRAM_ATTR onButtonISR() {
  uint32_t now = (uint32_t)(esp_timer_get_time() / 1000ULL);
  if (now - s_lastMs >= BUTTON_DEBOUNCE_MS) {
    s_lastMs  = now;
    s_pressed = true;
  }
}

void Button::begin(uint8_t pin) {
  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin), onButtonISR, FALLING);
}

bool Button::consumePress() {
  if (s_pressed) {
    s_pressed = false;
    return true;
  }
  return false;
}
