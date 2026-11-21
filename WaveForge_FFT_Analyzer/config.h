
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
//              HARDWARE PINS
// ==========================================

// Channel selection button
#define BUTTON_PIN 27

// Analog input pins
#define SQUARE_PIN   34
#define TRIANGLE_PIN 35
#define SINE_PIN     32

// ==========================================
//             CHANNEL SETTINGS
// ==========================================

#define CHANNEL_COUNT 3

// ==========================================
//             FFT PARAMETERS
// ==========================================

// Number of samples captured per analysis
#define FFT_SIZE 4096

// Sampling frequency in Hz
#define SAMPLING_FREQUENCY 8000

// Sampling interval in microseconds
#define SAMPLE_INTERVAL_US 125

// Number of harmonics used for THD calculation
#define HARMONIC_COUNT 10

// ==========================================
//              ADC SETTINGS
// ==========================================

#define ADC_RESOLUTION 12
#define ADC_MAX_VALUE 4095

// Square-wave input uses a 10k-10k divider
#define SQUARE_DIVIDER_GAIN 2.0f

// ==========================================
//             BUTTON SETTINGS
// ==========================================

#define DEBOUNCE_TIME_MS 150

#endif
