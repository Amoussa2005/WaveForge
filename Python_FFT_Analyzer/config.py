
# =====================================================
#              WAVEFORGE CONFIGURATION
# =====================================================

# SERIAL COMMUNICATION
SERIAL_PORT = "COM6"       # Change to your ESP32 COM port
BAUD_RATE = 921600
SERIAL_TIMEOUT = 2


# SIGNAL ACQUISITION
SAMPLING_FREQUENCY = 8000
FFT_SIZE = 4096

SAMPLE_INTERVAL = 1.0 / SAMPLING_FREQUENCY

# ADC CONFIGURATION
ADC_RESOLUTION = 12
ADC_MAX_VALUE = (2 ** ADC_RESOLUTION) - 1

ADC_REFERENCE_VOLTAGE = 3.3


# SIGNAL CHANNELS
CHANNEL_NAMES = {
    0: "Square",
    1: "Triangle",
    2: "Sine"
}

CHANNELS = {
    0: "Square",
    1: "Triangle",
    2: "Sine"
}

# Square input uses a 10k-10k voltage divider
SQUARE_VOLTAGE_GAIN = 2.0

TRIANGLE_VOLTAGE_GAIN = 1.0
SINE_VOLTAGE_GAIN = 1.0


# FFT CONFIGURATION
HARMONIC_COUNT = 10

MIN_FREQUENCY = 40
MAX_FREQUENCY = 500

DISPLAY_MAX_FREQUENCY = 1000


# DISPLAY CONFIGURATION
PLOT_UPDATE_INTERVAL = 100
