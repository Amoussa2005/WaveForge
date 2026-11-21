
#include <Arduino.h>

#include "config.h"
#include "SignalAcquisition.h"
#include "SerialPacket.h"

// =====================================================
//                    OBJECTS
// =====================================================

SignalAcquisition acquisition;

// =====================================================
//                CHANNEL MANAGEMENT
// =====================================================

uint8_t currentChannel = CHANNEL_SQUARE;

unsigned long lastButtonTime = 0;

bool previousButtonState = HIGH;

// =====================================================
//                  BUTTON CONTROL
// =====================================================

void updateChannel() {

    bool currentButtonState = digitalRead(BUTTON_PIN);

    // Detect button press
    if (previousButtonState == HIGH &&
        currentButtonState == LOW) {

        if (millis() - lastButtonTime >= DEBOUNCE_TIME_MS) {

            currentChannel++;

            if (currentChannel >= CHANNEL_COUNT) {
                currentChannel = CHANNEL_SQUARE;
            }

            lastButtonTime = millis();
        }
    }

    previousButtonState = currentButtonState;
}

// =====================================================
//                       SETUP
// =====================================================

void setup() {

    // Initialize binary serial communication
    initializePacketSerial();

    // Button configuration
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // ADC configuration
    analogReadResolution(ADC_RESOLUTION);

    const uint8_t signalPins[CHANNEL_COUNT] = {
        SQUARE_PIN,
        TRIANGLE_PIN,
        SINE_PIN
    };

    for (int i = 0; i < CHANNEL_COUNT; i++) {

        pinMode(signalPins[i], INPUT);

        analogSetPinAttenuation(
            signalPins[i],
            ADC_11db
        );

    }

}

// =====================================================
//                        LOOP
// =====================================================

void loop() {

    // Check whether the user selected another waveform
    updateChannel();

    // Capture 4096 calibrated samples
    bool captureSuccessful =
        acquisition.captureSamples(currentChannel);

    if (!captureSuccessful) {
        return;
    }

    // Transmit original calibrated millivolt samples
    sendADCPacket(
        currentChannel,
        acquisition.getSamplesMillivolts(),
        FFT_SIZE
    );

}
