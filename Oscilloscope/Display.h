#pragma once
#include <Arduino.h>
#include "Config.h"
#include "Sampler.h"

class Display {
public:
  bool begin();
  void splash();
  void drawScope(const ChannelConfig& ch, const Capture& cap);
private:
  void drawGrid();
  void drawHeader(const ChannelConfig& ch, const Capture& cap);
  void drawTrace(const Capture& cap);
};
