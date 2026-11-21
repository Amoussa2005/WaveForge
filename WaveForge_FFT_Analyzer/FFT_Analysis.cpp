
#include "FFT_Analysis.h"
#include <math.h>

// ==========================================
//                CONSTRUCTOR
// ==========================================

FFT_Analysis::FFT_Analysis()
    : FFT(real, imaginary, FFT_SIZE, SAMPLING_FREQUENCY) {

    fundamentalFrequency = 0.0f;
    fundamentalAmplitude = 0.0f;

    for (int i = 0; i < FFT_SIZE; i++) {
        real[i] = 0.0f;
        imaginary[i] = 0.0f;
    }
}

// ==========================================
//                HANN WINDOW
// ==========================================

void FFT_Analysis::applyHannWindow() {

    for (int i = 0; i < FFT_SIZE; i++) {

        float window = 0.5f * (
            1.0f - cosf(
                (2.0f * PI * i) / (FFT_SIZE - 1)
            )
        );

        real[i] *= window;
    }
}

// ==========================================
//                 FFT PROCESSING
// ==========================================

void FFT_Analysis::performFFT(float* inputSamples) {

    // Copy acquired samples into FFT arrays
    for (int i = 0; i < FFT_SIZE; i++) {

        real[i] = inputSamples[i];
        imaginary[i] = 0.0f;
    }

    // Apply Hann window
    applyHannWindow();

    // Perform forward FFT
    FFT.compute(FFTDirection::Forward);

    // Convert complex values to magnitudes
    FFT.complexToMagnitude();

    // Find the fundamental frequency
    findFundamental();
}

// ==========================================
//          FUNDAMENTAL FREQUENCY
// ==========================================

void FFT_Analysis::findFundamental() {

    float largestMagnitude = 0.0f;

    uint16_t fundamentalBin = 0;

    // Ignore low-frequency components and DC
    const float MIN_FREQUENCY = 40.0f;
    const float MAX_FREQUENCY = 500.0f;

    uint16_t firstBin = (uint16_t)(
        MIN_FREQUENCY * FFT_SIZE / SAMPLING_FREQUENCY
    );

    uint16_t lastBin = (uint16_t)(
        MAX_FREQUENCY * FFT_SIZE / SAMPLING_FREQUENCY
    );

    for (uint16_t bin = firstBin; bin <= lastBin; bin++) {

        if (real[bin] > largestMagnitude) {

            largestMagnitude = real[bin];
            fundamentalBin = bin;
        }
    }

    if (fundamentalBin == 0) {

        fundamentalFrequency = 0.0f;
        fundamentalAmplitude = 0.0f;

        return;
    }

    // Frequency represented by the selected FFT bin
    fundamentalFrequency =
        (float)fundamentalBin *
        SAMPLING_FREQUENCY / FFT_SIZE;

    // Hann window coherent gain correction
    const float WINDOW_SUM = (FFT_SIZE - 1) / 2.0f;

    // Convert FFT magnitude into peak voltage
    fundamentalAmplitude =
        2.0f * real[fundamentalBin] / WINDOW_SUM;
}

// ==========================================
//               DATA ACCESS
// ==========================================

float FFT_Analysis::getFrequency(uint16_t bin) {

    if (bin > FFT_SIZE / 2) {
        return 0.0f;
    }

    return (float)bin *
           SAMPLING_FREQUENCY / FFT_SIZE;
}

float FFT_Analysis::getAmplitude(uint16_t bin) {

    if (bin == 0 || bin > FFT_SIZE / 2) {
        return 0.0f;
    }

    const float WINDOW_SUM = (FFT_SIZE - 1) / 2.0f;

    return 2.0f * real[bin] / WINDOW_SUM;
}

uint16_t FFT_Analysis::getBinCount() {

    return (FFT_SIZE / 2) + 1;
}

float FFT_Analysis::getFundamentalFrequency() {

    return fundamentalFrequency;
}

float FFT_Analysis::getFundamentalAmplitude() {

    return fundamentalAmplitude;
}
