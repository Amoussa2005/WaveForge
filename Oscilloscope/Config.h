#pragma once
#include <Arduino.h>

// ---------------- OLED (SSD1306 128x64, SPI) ----------------
#define OLED_MOSI 23
#define OLED_SCK  18
#define OLED_CS   5
#define OLED_DC   22
#define OLED_RST  4

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_SPI_HZ   8000000UL

// ---------------- Button ----------------
#define BUTTON_PIN         27      // push button between GPIO27 and GND
#define BUTTON_DEBOUNCE_MS 150

// ---------------- Display layout ----------------
#define HEADER_HEIGHT 11
#define PLOT_TOP      12
#define PLOT_BOTTOM   (SCREEN_HEIGHT - 1)

// ---------------- Acquisition ----------------
#define SAMPLE_COUNT        SCREEN_WIDTH  // one sample per pixel column
#define PRESCAN_SAMPLES     80
#define TRIGGER_TIMEOUT_MS  200
#define MIN_SIGNAL_MV       55            // below this peak-to-peak => "NO SIGNAL"
#define MIN_INTERVAL_US     200           // fastest time base
#define MAX_INTERVAL_US     4000          // slowest time base

// ---------------- Input channels ----------------

struct ChannelConfig {
  const char* name;             // shown in header
  uint8_t     pin;              // ADC input
  uint32_t    sampleIntervalUs; // initial time base (auto-adjusted at run time)
  float       inputScale;       // 2.0 if a 1:2 divider is used, 1.0 otherwise
};

static const ChannelConfig CHANNELS[] = {
  { "SQUARE",   34, 300, 2.0f },
  { "TRIANGLE", 35, 300, 1.0f },
  { "SINE",     32, 200, 1.0f },
};
static const uint8_t CHANNEL_COUNT = sizeof(CHANNELS) / sizeof(CHANNELS[0]);
