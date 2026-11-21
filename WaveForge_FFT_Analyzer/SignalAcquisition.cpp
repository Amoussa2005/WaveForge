
#include "SignalAcquisition.h"
#include <math.h>

// ==========================================
//             CONSTRUCTOR
// ==========================================

SignalAcquisition::SignalAcquisition() {

    dcOffset = 0.0f;
    minimum = 0.0f;
    maximum = 0.0f;

}

// ==========================================
//           CAPTURE SAMPLES
// ==========================================

bool SignalAcquisition::captureSamples(uint8_t channel) {

    // Validate selected channel
    if (channel >= CHANNEL_COUNT) {
        return false;
    }

    // Signal input pins
    const uint8_t signalPins[CHANNEL_COUNT] = {
        SQUARE_PIN,
        TRIANGLE_PIN,
        SINE_PIN
    };

    uint8_t selectedPin = signalPins[channel];

    // Initialize measurements
    float sum = 0.0f;

    minimum = 1000000.0f;
    maximum = -1000000.0f;

    // Sampling timer
    uint32_t previousTime = micros();

    // ======================================
    //          SAMPLING LOOP
    // ======================================

    for (int i = 0; i < FFT_SIZE; i++) {

        // Maintain the requested sampling interval
        while ((uint32_t)(micros() - previousTime) < SAMPLE_INTERVAL_US) {
            // Wait
        }

        previousTime += SAMPLE_INTERVAL_US;

        // Read calibrated voltage in millivolts
        uint32_t millivolts = analogReadMilliVolts(selectedPin);

        // Correct square-wave voltage divider
        if (channel == 0) {

            millivolts = (uint32_t)lroundf(
                millivolts * SQUARE_DIVIDER_GAIN
            );

        }

        // Protect the uint16_t buffer
        if (millivolts > 65535) {
            millivolts = 65535;
        }

        // Store calibrated millivolts for Python
        samplesMillivolts[i] = (uint16_t)millivolts;

        // Convert to volts for local processing
        float voltage = millivolts / 1000.0f;

        samples[i] = voltage;

        // Calculate measurements before DC removal
        sum += voltage;

        if (voltage < minimum) {
            minimum = voltage;
        }

        if (voltage > maximum) {
            maximum = voltage;
        }

    }

    // ======================================
    //             DC OFFSET
    // ======================================

    dcOffset = sum / FFT_SIZE;

    // Remove DC only from the local voltage buffer
    removeDCOffset();

    return true;

}

// ==========================================
//           REMOVE DC OFFSET
// ==========================================

void SignalAcquisition::removeDCOffset() {

    for (int i = 0; i < FFT_SIZE; i++) {

        samples[i] -= dcOffset;

    }

}

// ==========================================
//              GETTERS
// ==========================================

float* SignalAcquisition::getSamples() {

    return samples;

}

uint16_t* SignalAcquisition::getSamplesMillivolts() {

    return samplesMillivolts;

}

float SignalAcquisition::getDCOffset() {

    return dcOffset;

}

float SignalAcquisition::getMinimum() {

    return minimum;

}

float SignalAcquisition::getMaximum() {

    return maximum;

}
