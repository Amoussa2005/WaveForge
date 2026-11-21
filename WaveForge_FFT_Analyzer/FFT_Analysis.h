
#ifndef FFT_ANALYSIS_H
#define FFT_ANALYSIS_H

#include <Arduino.h>
#include <arduinoFFT.h>
#include "config.h"

// ==========================================
//               FFT ANALYSIS CLASS
// ==========================================

class FFT_Analysis {

public:

    // Constructor
    FFT_Analysis();

    // Perform FFT on acquired samples
    void performFFT(float* inputSamples);

    // Get frequency of a specific FFT bin
    float getFrequency(uint16_t bin);

    // Get amplitude of a specific FFT bin
    float getAmplitude(uint16_t bin);

    // Get number of available frequency bins
    uint16_t getBinCount();

    // Get fundamental frequency
    float getFundamentalFrequency();

    // Get fundamental peak amplitude
    float getFundamentalAmplitude();

private:

    // Real and imaginary FFT arrays
    float real[FFT_SIZE];
    float imaginary[FFT_SIZE];

    // ArduinoFFT processor
    ArduinoFFT<float> FFT;

    // Fundamental measurements
    float fundamentalFrequency;
    float fundamentalAmplitude;

    // Apply Hann window
    void applyHannWindow();

    // Find fundamental frequency
    void findFundamental();

};

#endif
