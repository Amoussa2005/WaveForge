
#ifndef SERIAL_PACKET_H
#define SERIAL_PACKET_H

#include <Arduino.h>

// ================================================
//              PACKET CONFIGURATION
// ================================================

#define PACKET_HEADER_1 'W'
#define PACKET_HEADER_2 'F'

#define PACKET_SAMPLE_COUNT 4096

#define CHANNEL_SQUARE   0
#define CHANNEL_TRIANGLE 1
#define CHANNEL_SINE     2

// ================================================
//              FUNCTION DECLARATIONS
// ================================================

// Initialize serial communication
void initializePacketSerial();

// Calculate CRC16-CCITT
uint16_t calculateCRC16(
    const uint8_t* data,
    size_t length
);

// Transmit one complete ADC packet
void sendADCPacket(
    uint8_t channel,
    const uint16_t* samples,
    uint16_t sampleCountn
);

#endif
