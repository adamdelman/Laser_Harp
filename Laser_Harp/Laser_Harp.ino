#include <Stepper.h>
#include "Arduino.h"
#include <LaserHarp.h>
#include <MidiPlayer.h>

MidiPlayer *midi_player;
LaserHarp *laser_harp;
//The setup function is called once at startup of the sketch
Stepper *stepper_motor;

void setup() {
    Serial.begin(115200); //Use serial for debugging
    stepper_motor = new Stepper(48, A0, A2, A1, A3);
    midi_player = new MidiPlayer();
    laser_harp = new LaserHarp(midi_player, stepper_motor);
}

// The loop function is called in an endless loop
void loop() {
  laser_harp->loop();
}





















