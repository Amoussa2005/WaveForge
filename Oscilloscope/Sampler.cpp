
#include "Sampler.h"
#include <math.h>

// =====================================================
//                  INITIALIZATION
// =====================================================

void Sampler::begin() {

    analogReadResolution(12);

    for (uint8_t i = 0; i < CHANNEL_COUNT; i++) {

        pinMode(CHANNELS[i].pin, INPUT);

        analogSetPinAttenuation(
            CHANNELS[i].pin,
            ADC_11db
        );

        _intervalUs[i] = CHANNELS[i].sampleIntervalUs;
    }

}


// =====================================================
//                  ADC READING
// =====================================================

uint16_t Sampler::readMv(const ChannelConfig& ch) {

    uint32_t sum = 0;

    // Average four calibrated ADC readings
    for (uint8_t i = 0; i < 4; i++) {
        sum += analogReadMilliVolts(ch.pin);
    }

    float mv = (sum / 4.0f) * ch.inputScale;

    if (mv < 0.0f) mv = 0.0f;
    if (mv > 65535.0f) mv = 65535.0f;

    return (uint16_t)(mv + 0.5f);
}


// =====================================================
//             MEDIAN OF PERIOD MEASUREMENTS
// =====================================================

static float calculateMedian(float* values, uint8_t count) {

    if (count == 0) return 0.0f;

    // Sort the measurements
    for (uint8_t i = 0; i < count - 1; i++) {

        for (uint8_t j = i + 1; j < count; j++) {

            if (values[j] < values[i]) {

                float temp = values[i];
                values[i] = values[j];
                values[j] = temp;
            }
        }
    }

    if (count % 2 == 1) {
        return values[count / 2];
    }

    return (values[count / 2 - 1] +
            values[count / 2]) / 2.0f;
}


// =====================================================
//                  SIGNAL ACQUISITION
// =====================================================

void Sampler::capture(uint8_t idx, Capture& out) {

    if (idx >= CHANNEL_COUNT) {
        out.hasSignal = false;
        out.triggered = false;
        out.freqHz = 0.0f;
        out.minMv = 0;
        out.maxMv = 0;
        return;
    }

    const ChannelConfig& ch = CHANNELS[idx];

    uint32_t interval = _intervalUs[idx];


    // =================================================
    // 1. PRE-SCAN
    // =================================================

    uint16_t lo = 65535;
    uint16_t hi = 0;

    for (uint16_t i = 0; i < PRESCAN_SAMPLES; i++) {

        uint16_t v = readMv(ch);

        if (v < lo) lo = v;
        if (v > hi) hi = v;

        delayMicroseconds(interval);
    }

    int span = (int)hi - (int)lo;

    int mid = ((int)hi + (int)lo) / 2;

    // Hysteresis reduces false triggering from noise
    int hyst = span / 8;

    if (hyst < 5) hyst = 5;


    // =================================================
    // 2. RISING-EDGE TRIGGER
    // =================================================

    out.triggered = false;

    uint16_t firstSample = 0;

    uint32_t triggerTime = 0;

    if (span >= MIN_SIGNAL_MV) {

        uint32_t start = millis();

        bool armed = false;

        while ((uint32_t)(millis() - start) <
               TRIGGER_TIMEOUT_MS) {

            uint16_t v = readMv(ch);

            if ((int)v < mid - hyst) {

                armed = true;

            } else if (armed && (int)v >= mid) {

                firstSample = v;

                triggerTime = micros();

                out.triggered = true;

                break;
            }
        }
    }


    // =================================================
    // 3. SAMPLE ACQUISITION
    // =================================================

    uint32_t t0;

    if (out.triggered) {
        t0 = triggerTime;
    } else {
        t0 = micros();
    }

    uint32_t nextSampleTime = t0;

    for (uint16_t i = 0; i < SAMPLE_COUNT; i++) {

        if (i == 0 && out.triggered) {

            out.samples[i] = firstSample;

            _ts[i] = 0;

            continue;
        }

        if (i > 0 || !out.triggered) {
            nextSampleTime += interval;
        }

        while ((int32_t)(micros() - nextSampleTime) < 0) {
            // Wait for sampling instant
        }

        out.samples[i] = readMv(ch);

        // Record the actual sampling timestamp
        _ts[i] = micros() - t0;
    }


    // =================================================
    // 4. MINIMUM AND MAXIMUM
    // =================================================

    uint16_t mn = 65535;
    uint16_t mx = 0;

    for (uint16_t i = 0; i < SAMPLE_COUNT; i++) {

        if (out.samples[i] < mn) {
            mn = out.samples[i];
        }

        if (out.samples[i] > mx) {
            mx = out.samples[i];
        }
    }

    out.minMv = mn;
    out.maxMv = mx;

    span = (int)mx - (int)mn;

    out.hasSignal = span >= MIN_SIGNAL_MV;

    out.freqHz = 0.0f;


    // =================================================
    // 5. FREQUENCY DETECTION
    //    HYSTERESIS + INTERPOLATION
    // =================================================

    float periods[8];

    uint8_t periodCount = 0;

    if (out.hasSignal) {

        float midpoint =
            ((float)mx + (float)mn) / 2.0f;

        float hysteresis = span * 0.12f;

        if (hysteresis < 5.0f) {
            hysteresis = 5.0f;
        }

        float lowerThreshold = midpoint - hysteresis;

        bool armed = false;

        bool previousCrossingValid = false;

        float previousCrossingTime = 0.0f;

        for (uint16_t i = 1; i < SAMPLE_COUNT; i++) {

            float previousValue = out.samples[i - 1];

            float currentValue = out.samples[i];

            // Arm only after reaching the lower threshold
            if (currentValue < lowerThreshold) {

                armed = true;
            }

            // Detect a rising crossing
            else if (armed &&
                     previousValue < midpoint &&
                     currentValue >= midpoint) {

                float difference =
                    currentValue - previousValue;

                if (difference <= 0.0f) {
                    armed = false;
                    continue;
                }

                // Interpolate the crossing time
                float fraction =
                    (midpoint - previousValue) / difference;

                if (fraction < 0.0f) fraction = 0.0f;
                if (fraction > 1.0f) fraction = 1.0f;

                float crossingTime =
                    _ts[i - 1] +
                    fraction * (_ts[i] - _ts[i - 1]);

                if (previousCrossingValid) {

                    float measuredPeriod =
                        crossingTime - previousCrossingTime;

                    // Accept only periods in the configured
                    // frequency range.
                    if (measuredPeriod > 0.0f) {

                        float measuredHz =
                            1000000.0f / measuredPeriod;

                        if (measuredHz >= 1.0f &&
                            measuredHz <= 10000.0f) {

                            if (periodCount < 8) {

                                periods[periodCount++] =
                                    measuredPeriod;

                            } else {

                                // Keep the most recent periods
                                for (uint8_t k = 0; k < 7; k++) {
                                    periods[k] = periods[k + 1];
                                }

                                periods[7] = measuredPeriod;
                            }
                        }
                    }
                }

                previousCrossingTime = crossingTime;

                previousCrossingValid = true;

                armed = false;
            }
        }
    }


    // =================================================
    // 6. ROBUST FREQUENCY CALCULATION
    // =================================================

    float medianPeriod = 0.0f;

    if (periodCount >= 2) {

        medianPeriod =
            calculateMedian(periods, periodCount);

        if (medianPeriod > 0.0f) {

            out.freqHz =
                1000000.0f / medianPeriod;
        }
    }


    // =================================================
    // 7. AUTOMATIC TIME-BASE ADJUSTMENT
    // =================================================

    if (out.freqHz > 0.0f) {

        float periodUs = 1000000.0f / out.freqHz;

        // Target approximately 32 samples per cycle
        uint32_t newInterval =
            (uint32_t)(periodUs / 32.0f);

        if (newInterval < MIN_INTERVAL_US) {
            newInterval = MIN_INTERVAL_US;
        }

        if (newInterval > MAX_INTERVAL_US) {
            newInterval = MAX_INTERVAL_US;
        }

        // Avoid unnecessary small changes
        float ratio =
            (float)newInterval / (float)interval;

        if (ratio < 0.80f || ratio > 1.20f) {
            _intervalUs[idx] = newInterval;
        }

    } else {

        // If the signal is too slow to show enough cycles,
        // gradually increase the sampling interval.
        if (out.hasSignal && periodCount == 0) {

            uint32_t newInterval = interval * 2;

            if (newInterval > MAX_INTERVAL_US) {
                newInterval = MAX_INTERVAL_US;
            }

            _intervalUs[idx] = newInterval;
        }
    }


    // =================================================
    // 8. SERIAL DIAGNOSTICS
    // =================================================

    Serial.println();
    Serial.println("========== SIGNAL DATA ==========");

    Serial.print("Channel: ");
    Serial.println(ch.name);

    Serial.print("ADC Pin: GPIO");
    Serial.println(ch.pin);

    Serial.print("Minimum (mV): ");
    Serial.println(mn);

    Serial.print("Maximum (mV): ");
    Serial.println(mx);

    Serial.print("Peak-to-Peak (mV): ");
    Serial.println(span);

    Serial.print("DC Midpoint (mV): ");
    Serial.println(((uint32_t)mn + mx) / 2);

    Serial.print("Triggered: ");
    Serial.println(out.triggered ? "YES" : "NO");

    Serial.print("Valid periods: ");
    Serial.println(periodCount);

    Serial.print("Frequency: ");

    if (out.freqHz > 0.0f) {

        Serial.print(out.freqHz, 2);
        Serial.println(" Hz");

    } else {

        Serial.println("Not detected");
    }

    Serial.print("Requested sampling interval (us): ");
    Serial.println(interval);

    Serial.print("Next sampling interval (us): ");
    Serial.println(_intervalUs[idx]);

    if (SAMPLE_COUNT > 1) {

        float actualInterval =
            (float)_ts[SAMPLE_COUNT - 1] /
            (SAMPLE_COUNT - 1);

        Serial.print("Actual average interval (us): ");
        Serial.println(actualInterval, 1);
    }

    Serial.println("=================================");
}
