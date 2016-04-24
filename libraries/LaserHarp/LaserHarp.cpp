#include "LaserHarp.h"

LaserHarp::LaserHarp(MidiPlayer *midiplayer, Stepper *stepper_motor) : m_midiplayer{midiplayer},
                                                                       m_stepper_motor{stepper_motor} {

    pinMode((uint8_t) Pins::laser, OUTPUT);            // Setup for laser.
    pinMode((uint8_t) Pins::led, OUTPUT);              // Setup for status led.
    m_stepper_motor->setSpeed(120);
}

void LaserHarp::handle_note(int note_number) {
    Serial.print("Handling note: ");
    Serial.println(note_number);
    if ((analogRead(0) > sensor_threshold) &&
        (!m_notes_active_status[note_number])) // If the sensor_threshold gets a signal, and the note is not playing:
    {
        digitalWrite((uint8_t) Pins::led, HIGH);       // Switch on status led.
        m_midiplayer->noteOn(Notes[note_number], 0x7F);
        m_notes_active_status[note_number] = true;
    }


    else if (analogRead(0) < sensor_threshold)             // If the sensor_threshold does not get a signal:

    {
        digitalWrite((uint8_t) Pins::led, LOW);         // Switch off the status led.
        m_midiplayer->noteOff(Notes[note_number], 0x00);     // Stop playing note 1.
        m_notes_active_status[note_number] = false;
    }
}

void LaserHarp::handle_beam(int beam_number, int last_beam) {
    Serial.print("Handling beam: ");
    Serial.println(beam_number);
    Serial.print("Last beam: ");
    Serial.println(last_beam);
    digitalWrite((uint8_t) Pins::laser, HIGH);               // Turn on the laser for the 1st beam.
    delay(laser_delay);
    handle_note(beam_number);
    digitalWrite((uint8_t) Pins::laser, LOW);

    if (beam_number > last_beam) {
        Serial.println("Moving one step forward");
        m_stepper_motor->step(m_motor_step_size);
    }
    else {
        Serial.println("Moving one step backward");
        m_stepper_motor->step(-m_motor_step_size);
    }
//    delay(50);
}


void LaserHarp::loop() {
    Serial.println("Started going forward.");
    int last_beam = -1;

    for (int beam_number = 0; beam_number < m_beam_count; beam_number++) {
        handle_beam(beam_number, last_beam);
        last_beam = beam_number;
    }
    Serial.println("Finished going forward.");
    last_beam = 9;
    for (int beam_number = m_beam_count; beam_number > 0; beam_number--) {
        handle_beam(beam_number, last_beam);
    }
}

