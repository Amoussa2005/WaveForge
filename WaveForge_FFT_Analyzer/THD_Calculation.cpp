
#include "THD_Calculation.h"
#include <math.h>

// ==========================================
//                CONSTRUCTOR
// ==========================================

THD_Calculation::THD_Calculation() {
}

// ==========================================
//             HARMONIC BIN SEARCH
// ==========================================

uint16_t THD_Calculation::findHarmonicBin(
    FFT_Analysis& fft,
    float targetFrequency
) {

    const float NYQUIST_FREQUENCY =
        SAMPLING_FREQUENCY / 2.0f;

    // Reject frequencies outside the FFT range
    if (targetFrequency >= NYQUIST_FREQUENCY ||
        targetFrequency <= 0.0f) {
        return 0;
    }

    // Convert target frequency into an FFT bin
    uint16_t centerBin = (uint16_t)roundf(
        targetFrequency * FFT_SIZE /
        SAMPLING_FREQUENCY
    );

    // Search approximately +/- 2 bins
    const int SEARCH_RADIUS = 2;

    uint16_t bestBin = 0;
    float largestAmplitude = 0.0f;

    int firstBin = max(1, (int)centerBin - SEARCH_RADIUS);

    int lastBin = min(
        (int)(FFT_SIZE / 2) - 1,
        (int)centerBin + SEARCH_RADIUS
    );

    for (int bin = firstBin; bin <= lastBin; bin++) {

        float amplitude = fft.getAmplitude(bin);

        if (amplitude > largestAmplitude) {

            largestAmplitude = amplitude;
            bestBin = bin;
        }
    }

    return bestBin;
}

// ==========================================
//               THD CALCULATION
// ==========================================

THDResults THD_Calculation::calculate(
    FFT_Analysis& fft
) {

    THDResults results = {};

    results.valid = false;
    results.thdPercentage = 0.0f;

    // Get fundamental measurements
    results.fundamentalFrequency =
        fft.getFundamentalFrequency();

    results.fundamentalAmplitude =
        fft.getFundamentalAmplitude();

    // Validate fundamental amplitude
    if (results.fundamentalFrequency <= 0.0f ||
        results.fundamentalAmplitude <= 0.000001f) {

        return results;
    }

    float harmonicSum = 0.0f;

    // ======================================
    //       MEASURE HARMONICS 2 TO 10
    // ======================================

    for (int harmonic = 2;
         harmonic <= HARMONIC_COUNT;
         harmonic++) {

        int index = harmonic - 2;

        // Expected harmonic frequency
        float targetFrequency =
            harmonic * results.fundamentalFrequency;

        results.harmonicFrequencies[index] =
            targetFrequency;

        results.harmonicAmplitudes[index] = 0.0f;

        // Find the nearest strong FFT bin
        uint16_t bin = findHarmonicBin(
            fft,
            targetFrequency
        );

        // Skip harmonics outside measurable range
        if (bin == 0) {
            continue;
        }

        // Extract harmonic peak amplitude
        float amplitude = fft.getAmplitude(bin);

        results.harmonicAmplitudes[index] = amplitude;

        // Add squared harmonic amplitude
        harmonicSum += amplitude * amplitude;
    }

    // ======================================
    //               FINAL THD
    // ======================================

    float harmonicRMS = sqrtf(harmonicSum);

    results.thdPercentage =
        100.0f * harmonicRMS /
        results.fundamentalAmplitude;

    results.valid = true;

    return results;
}
