#include <Arduino.h>
#include <FastLED.h>


#define TICKS_PER_SECOND 120

#define ANIMATION_SINELON_NUM_COLORS 3

#define PIN_LEDS 15
#define NUM_LEDS 11


CRGB leds[NUM_LEDS];
void (*animation)() = NULL;

CRGB animation_sinelon_colors[] = { CRGB(0, 16, 32), CRGB(32, 0, 16), CRGB(24, 24, 24) };


void animation_sinelon() {
    int i, pos, phase;
    
    // Gradually fade to black
    fadeToBlackBy(leds, NUM_LEDS, 20);

    // Set a different color ad multiple points
    for (i = 0; i < ANIMATION_SINELON_NUM_COLORS; i++) {
        phase = UINT16_MAX / ANIMATION_SINELON_NUM_COLORS * i;
        pos = beatsin16(20, 0, NUM_LEDS - 1, 0, phase);

        // Add the color for a smoother animation
        leds[pos] += animation_sinelon_colors[i];
    }
}


void setup() {
    Serial.begin(115200);

    // Setup the led strip
    FastLED.addLeds<NEOPIXEL, PIN_LEDS>(leds, NUM_LEDS);
    animation = NULL;
}

void loop() {
    // Play any active animation
    if (animation) animation();

    // IR sensor test
    int value = 255 - analogRead(32) * 255 / 4095;
    fill_solid(leds, NUM_LEDS, CHSV(200, 200, value));

    // Update and delay
    FastLED.show();
    FastLED.delay(1000 / TICKS_PER_SECOND);
}
