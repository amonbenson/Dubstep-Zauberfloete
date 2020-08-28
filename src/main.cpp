#include <Arduino.h>
#include <Bounce2.h>
#include <FastLED.h>
#include <Midi.h>

#include "palette.h"


// Led strip
#define LEDS_COUNT 11
#define LEDS_PIN 15

// Air sensor
#define AIR_PIN 34

#define AIR_DEBOUNCE_INTERVAL 50

// Holes
#define HOLES_COUNT 5
#define HOLES_PINS { 33, 25, 12, 27, 14 }
#define HOLES_AMPLIFICATION { 5.0, 8.0, 20.0, 5.0, 5.0 }
#define HOLES_OFFSET { 0.1, 0.1, 0.1, 0.1, 0.1 }
#define HOLES_ACTIVE_THRESH 0.9

#define HOLES_CALIBRATION_DELAY 30
#define HOLES_CALIBRATION_ITERATIONS 5

// Midi
#define MIDI_ROOTNOTE 36 // C2

// General settings
#define ANALOG_READ_MAX 4095



CRGB leds[LEDS_COUNT];


Bounce air = Bounce();
bool airFlowing = false;


const int holesPins[] = HOLES_PINS;
const float holesAmplification[] = HOLES_AMPLIFICATION;
const float holesOffset[] = HOLES_OFFSET;

int holesThresh[HOLES_COUNT];
bool holes[HOLES_COUNT];


MIDI_CREATE_DEFAULT_INSTANCE();
midi::DataByte currentNote = 0;



void calibrateHoles() {
    float value;

    for (int i = 0; i < HOLES_COUNT; i++) {
        // Take a few readings
        value = 0;
        for (int j = 0; j < HOLES_CALIBRATION_ITERATIONS; j++) {
            value += analogRead(holesPins[i]) / (float) HOLES_CALIBRATION_ITERATIONS;
            delay(HOLES_CALIBRATION_DELAY);
        }

        // Set the threshold
        holesThresh[i] = (int) value;
    }
}

void updateHoles() {
    // Update the holes
    float value;
    bool hole, recalculateNote;
    for (int i = 0; i < HOLES_COUNT; i++) {
        // Read the value
        value = (holesThresh[i] - analogRead(holesPins[i])) / (float) ANALOG_READ_MAX * holesAmplification[i] - holesOffset[i];
        if (value < 0) value = 0;
        if (value > 1) value = 1;

        // Set the hole value
        hole = value > HOLES_ACTIVE_THRESH;
        holes[i] = hole;

        // TODO: Check if the note changes while playing
    }
}

void updateCurrentNote() {
    //  Binary encode the first 3 holes to get the octave and the lower 2 for the note
    midi::DataByte octave = holes[0] | holes[1] << 1 | holes[2] << 2;
    midi::DataByte note = holes[3] | holes[4] << 1;

    // Update the current note
    currentNote = MIDI_ROOTNOTE + octave * 12 + note;
}


void handleNoteOn(byte channel, byte note, byte velocity) {
    // Get the led index
    int led = note - MIDI_ROOTNOTE;
    if (led < 0) led = 0;
    if (led >= LEDS_COUNT) led = LEDS_COUNT - 1;

    // Get the color from the palette
    leds[led] = colorPalette[velocity];
}

void handleNoteOff(byte channel, byte note, byte velocity) {
    // Turn the led off (velocity 0)
    handleNoteOn(channel, note, 0);
}


void setup() {
    // Turn off the buildin led
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    // Start midi with custom baud rate
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    Serial.begin(115200);
    currentNote = 0;

    // Setup the led strip
    FastLED.addLeds<NEOPIXEL, LEDS_PIN>(leds, LEDS_COUNT);

    // Setup the air sensor
    air.attach(AIR_PIN, INPUT);
    air.interval(0);

    // Setup the holes
    for (int i = 0; i < HOLES_COUNT; i++) {
        pinMode(holesPins[i], INPUT);
    }

    // Start calibrating
    calibrateHoles();
}

void loop() {
    // Check which holes are covered
    updateHoles();
    
    // Check if air is flowing
    air.update();
    if (air.fell()) {
        // Once air is flowing, activate the debouncing
        air.interval(AIR_DEBOUNCE_INTERVAL);
        airFlowing = true;

        // Start playing
        updateCurrentNote();
        MIDI.sendNoteOn(currentNote, 127, 1);
    } else if (air.rose()) {
        // If the air flow is done, deactivate debouncing by setting the interval to zero
        air.interval(0);
        airFlowing = false;

        // Stop playing
        MIDI.sendNoteOn(currentNote, 0, 1);
    }
    
    // Read any incoming midi messages
    MIDI.read();

    // Update
    FastLED.show();
}
