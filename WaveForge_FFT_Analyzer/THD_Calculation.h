
#ifndef THD_CALCULATION_H
#define THD_CALCULATION_H

#include <Arduino.h>
#include "config.h"
#include "FFT_Analysis.h"

// ==========================================
//              THD RESULTS
// ==========================================

struct THDResults {

    float fundamentalFrequency;

    float fundamentalAmplitude;

    float harmonicAmplitudes[HARMONIC_COUNT];

    float harmonicFrequencies[HARMONIC_COUNT];

    float thdPercentage;

    bool valid;

};

// ==========================================
//             THD CALCULATION CLASS
// ==========================================

class THD_Calculation {

public:

    // Constructor
    THD_Calculation();

    // Calculate THD from FFT spectrum
    THDResults calculate(FFT_Analysis& fft);

private:

    // Find the strongest FFT bin near a target frequency
    uint16_t findHarmonicBin(
        FFT_Analysis& fft,
        float targetFrequency
    );

};

#endif
