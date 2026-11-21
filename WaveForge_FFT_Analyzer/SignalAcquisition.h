
#ifndef SIGNAL_ACQUISITION_H
#define SIGNAL_ACQUISITION_H

#include <Arduino.h>
#include "config.h"

// ==========================================
//           SIGNAL ACQUISITION CLASS
// ==========================================

class SignalAcquisition {

public:

    SignalAcquisition();

    // Capture samples from selected channel
    bool captureSamples(uint8_t channel);

    // Samples in volts, with DC removed
    float* getSamples();

    // Original calibrated samples in millivolts
    uint16_t* getSamplesMillivolts();

    // Measurements
    float getDCOffset();

    float getMinimum();
    float getMaximum();

private:

    // Voltage samples used for signal processing
    float samples[FFT_SIZE];

    // Calibrated millivolt samples for Python
    uint16_t samplesMillivolts[FFT_SIZE];

    // Average voltage before DC removal
    float dcOffset;

    float minimum;
    float maximum;

    void removeDCOffset();

};

#endif
