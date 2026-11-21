
#include "SerialPacket.h"

// ================================================
//              SERIAL INITIALIZATION
// ================================================

void initializePacketSerial() {

    Serial.begin(921600);

    // Allow the USB serial connection to initialize
    delay(500);

}


// ================================================
//              CRC16-CCITT
// ================================================

uint16_t calculateCRC16(
    const uint8_t* data,
    size_t length
) {

    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; i++) {

        crc ^= ((uint16_t)data[i] << 8);

        for (uint8_t bit = 0; bit < 8; bit++) {

            if (crc & 0x8000) {

                crc = (crc << 1) ^ 0x1021;

            } else {

                crc <<= 1;

            }

        }

    }

    return crc;
}


// ================================================
//              SEND ADC PACKET
// ================================================

void sendADCPacket(
    uint8_t channel,
    const uint16_t* samples,
    uint16_t sampleCount
) {

    // Validate input
    if (samples == nullptr) {
        return;
    }

    if (channel > CHANNEL_SINE) {
        return;
    }

    if (sampleCount != PACKET_SAMPLE_COUNT) {
        return;
    }

    // --------------------------------------------
    // Build metadata
    // --------------------------------------------

    uint8_t metadata[3];

    metadata[0] = channel;

    metadata[1] = sampleCount & 0xFF;

    metadata[2] = (sampleCount >> 8) & 0xFF;


    // --------------------------------------------
    // Calculate CRC
    // CRC covers metadata + ADC payload
    // --------------------------------------------

    uint16_t crc = 0xFFFF;

    // Process metadata
    crc = calculateCRC16(metadata, sizeof(metadata));

    // Continue CRC over ADC samples
    // Samples are transmitted little-endian

    for (uint16_t i = 0; i < sampleCount; i++) {

        uint8_t lowByte = samples[i] & 0xFF;

        uint8_t highByte = (samples[i] >> 8) & 0xFF;

        uint8_t sampleBytes[2] = {
            lowByte,
            highByte
        };

        // Continue the CRC calculation
        for (uint8_t j = 0; j < 2; j++) {

            crc ^= ((uint16_t)sampleBytes[j] << 8);

            for (uint8_t bit = 0; bit < 8; bit++) {

                if (crc & 0x8000) {

                    crc = (crc << 1) ^ 0x1021;

                } else {

                    crc <<= 1;

                }

            }

        }

    }


    // --------------------------------------------
    // Transmit packet
    // --------------------------------------------

    // Header
    Serial.write(PACKET_HEADER_1);
    Serial.write(PACKET_HEADER_2);

    // Metadata
    Serial.write(metadata, sizeof(metadata));

    // ADC samples
    for (uint16_t i = 0; i < sampleCount; i++) {

        uint8_t sampleBytes[2];

        sampleBytes[0] = samples[i] & 0xFF;

        sampleBytes[1] = (samples[i] >> 8) & 0xFF;

        Serial.write(sampleBytes, 2);

    }

    // CRC (little-endian)
    Serial.write((uint8_t)(crc & 0xFF));

    Serial.write((uint8_t)(crc >> 8));

}
