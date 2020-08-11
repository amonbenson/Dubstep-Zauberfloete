#include <Arduino.h>

#include <BLEMidi.h>
#include <FastLED.h>


#define MIDI_DEVICE_NAME "Dubstep Zauberfloete"

#define LEDS_PIN 15
#define LEDS_LENGTH 14


BLEMIDI_CREATE_INSTANCE(midi);
CRGB leds[LEDS_LENGTH];


void setup() {
    Serial.begin(115200);

    // Setup the BLE Midi connection
    midi.begin(MIDI_DEVICE_NAME);
    FastLED.addLeds<NEOPIXEL, LEDS_PIN>(leds, LEDS_LENGTH);
}

void loop() {
    Serial.println("Sending Note");

    // Note On
    midi.sendNoteOn(60, 127, 1);
    leds[0] = CRGB::Blue;
    FastLED.show();
    delay(500);

    // Note Off
    midi.sendNoteOn(60, 0, 1);
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(500);
}
