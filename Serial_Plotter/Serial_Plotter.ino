
#include <Arduino.h>

// ========================================
//              PIN CONFIGURATION
// ========================================

#define BUTTON_PIN 27

#define SQUARE_PIN   34
#define TRIANGLE_PIN 35
#define SINE_PIN     32

// ========================================
//             OSCILLOSCOPE
// ========================================

#define SAMPLE_INTERVAL_US 250
#define DEBOUNCE_MS        150

const uint8_t signalPins[] = {
  SQUARE_PIN,
  TRIANGLE_PIN,
  SINE_PIN
};

const char* channelNames[] = {
  "SQUARE",
  "TRIANGLE",
  "SINE"
};

const uint8_t CHANNEL_COUNT = 3;

uint8_t currentChannel = 0;

unsigned long lastButtonTime = 0;
unsigned long lastSampleTime = 0;

// ========================================
//                  SETUP
// ========================================

void setup() {

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(12);

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    pinMode(signalPins[i], INPUT);
    analogSetPinAttenuation(signalPins[i], ADC_11db);
  }

  delay(1000);

  Serial.println("# WaveForge Laptop Oscilloscope");
  Serial.println("# Starting with SQUARE");
}

// ========================================
//             BUTTON HANDLING
// ========================================

void updateChannel() {

  static bool previousButtonState = HIGH;

  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (previousButtonState == HIGH &&
      currentButtonState == LOW) {

    if (millis() - lastButtonTime >= DEBOUNCE_MS) {

      currentChannel++;

      if (currentChannel >= CHANNEL_COUNT) {
        currentChannel = 0;
      }

      lastButtonTime = millis();

      Serial.print("# CHANNEL: ");
      Serial.println(channelNames[currentChannel]);
    }
  }

  previousButtonState = currentButtonState;
}

// ========================================
//              MAIN LOOP
// ========================================

void loop() {

  updateChannel();

  unsigned long now = micros();

  if (now - lastSampleTime >= SAMPLE_INTERVAL_US) {

    lastSampleTime = now;

    uint16_t voltage = analogReadMilliVolts(
  signalPins[currentChannel]
  );

  Serial.println(voltage);
  }
}
