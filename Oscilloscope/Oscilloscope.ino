
#include "Config.h"
#include "Button.h"
#include "Sampler.h"
#include "Display.h"

static Button button;
static Sampler sampler;
static Display display;
static Capture capture;

static uint8_t current = 0;


// =====================================================
//                       SETUP
// =====================================================

void setup() {

    // Initialize Serial Monitor
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("     WAVEFORGE OSCILLOSCOPE");
    Serial.println("================================");

    // Initialize button
    button.begin(BUTTON_PIN);

    // Initialize sampler
    sampler.begin();

    // Initialize OLED
    if (!display.begin()) {

        Serial.println("ERROR: OLED initialization failed!");

        while (true) {
            delay(1000);
        }
    }

    display.splash();

    Serial.println("OLED initialized successfully.");
    Serial.println("Sampler initialized successfully.");
    Serial.println("Serial diagnostics enabled.");
    Serial.println("================================");
}


// =====================================================
//                        LOOP
// =====================================================

void loop() {

    // Check channel selection
    if (button.consumePress()) {

        current = (current + 1) % CHANNEL_COUNT;

        Serial.println();
        Serial.print("Selected channel: ");
        Serial.println(CHANNELS[current].name);
    }

    // Acquire waveform
    sampler.capture(current, capture);

    // Display waveform
    display.drawScope(CHANNELS[current], capture);

}
