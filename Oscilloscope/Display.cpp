#include "Display.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI,
                             OLED_DC, OLED_RST, OLED_CS, OLED_SPI_HZ);

bool Display::begin() {
  SPI.begin(OLED_SCK, -1, OLED_MOSI, -1);
  if (!oled.begin(SSD1306_SWITCHCAPVCC)) return false;
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.display();
  return true;
}

void Display::splash() {
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setCursor(10, 14);
  oled.print("WaveForge");
  oled.setTextSize(1);
  oled.setCursor(22, 40);
  oled.print("Oscilloscope");
  oled.display();
  delay(1200);
}

void Display::drawGrid() {
  int midY = (PLOT_TOP + PLOT_BOTTOM) / 2;
  for (int x = 0; x < SCREEN_WIDTH; x += 4) oled.drawPixel(x, midY, SSD1306_WHITE);
  for (int gx = 32; gx < SCREEN_WIDTH; gx += 32)
    for (int y = PLOT_TOP; y <= PLOT_BOTTOM; y += 4) oled.drawPixel(gx, y, SSD1306_WHITE);
}

void Display::drawHeader(const ChannelConfig& ch, const Capture& cap) {
  char fbuf[14];
  if (cap.freqHz <= 0)         snprintf(fbuf, sizeof(fbuf), "---");
  else if (cap.freqHz >= 1000) snprintf(fbuf, sizeof(fbuf), "%.2f kHz", cap.freqHz / 1000.0f);
  else                         snprintf(fbuf, sizeof(fbuf), "%.1f Hz", cap.freqHz);

  oled.setTextSize(1);
  oled.setCursor(0, 1);
  oled.print(ch.name);

  int w = strlen(fbuf) * 6;                 // right-aligned frequency
  oled.setCursor(SCREEN_WIDTH - w, 1);
  oled.print(fbuf);

  oled.drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, SSD1306_WHITE);
}

void Display::drawTrace(const Capture& cap) {
  const int h = PLOT_BOTTOM - PLOT_TOP;

  if (!cap.hasSignal) {
    int midY = (PLOT_TOP + PLOT_BOTTOM) / 2;
    oled.drawFastHLine(0, midY, SCREEN_WIDTH, SSD1306_WHITE);
    oled.setCursor(34, midY - 12);
    oled.print("NO SIGNAL");
    return;
  }

  // Auto-scale vertically with a small margin
  float span  = (float)(cap.maxMv - cap.minMv);
  float lo    = cap.minMv - span * 0.08f;
  float scale = h / (span * 1.16f);

  int prevX = 0, prevY = 0;
  for (int x = 0; x < SAMPLE_COUNT; x++) {
    int y = PLOT_BOTTOM - (int)((cap.samples[x] - lo) * scale);
    y = constrain(y, PLOT_TOP, PLOT_BOTTOM);
    if (x > 0) oled.drawLine(prevX, prevY, x, y, SSD1306_WHITE);
    prevX = x;
    prevY = y;
  }
}

void Display::drawScope(const ChannelConfig& ch, const Capture& cap) {
  oled.clearDisplay();
  drawGrid();
  drawTrace(cap);
  drawHeader(ch, cap);
  oled.display();
}
