#pragma once
#include <Arduino.h>
#include "Config.h"

struct Capture {
  uint16_t samples[SAMPLE_COUNT];  // millivolts at the input (after scaling)
  uint16_t minMv;
  uint16_t maxMv;
  float    freqHz;                 // 0 if not measurable
  bool     triggered;
  bool     hasSignal;
};

class Sampler {
public:
  void begin();
  void capture(uint8_t channelIndex, Capture& out);
private:
  uint16_t readMv(const ChannelConfig& ch);
  uint32_t _intervalUs[CHANNEL_COUNT];
  uint32_t _ts[SAMPLE_COUNT];      // real sample timestamps (us)
};
