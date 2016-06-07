#include "LaserHarp.h"

#define CALIBRATION_ROUND_COUNT 10


LaserHarp::LaserHarp() {
    m_instrument = MIDI_GM1_INSTRUMENTS::Whistle;
    m_stepper_motor = new Stepper(48, A0, A2, A1, A3, false);
    m_stepper_motor->setSpeed(90);
    m_midiplayer = new MidiPlayer();
    m_midiplayer->enable_melodic_bank();
    m_midiplayer->vs_set_volume(50,10);
    m_midiplayer->set_channel_reverb(0, 5);
    m_midiplayer->vs_set_reverb(1);
    m_midiplayer->set_instrument(m_instrument);

    pinMode((uint8_t) Pins::laser, OUTPUT);            // Setup for laser.

    digitalWrite((uint8_t) Pins::laser, HIGH);
    calibrate_light_sensor();
    pinMode(Pins::button, INPUT_PULLUP);
}

void LaserHarp::calibrate_light_sensor() {
    delay(100);
    int light_level = 0;
    pinMode((uint8_t) laser, OUTPUT);            // Setup for laser.
    for (int counter = 0; counter < CALIBRATION_ROUND_COUNT; counter++) {
        delay(100);
        m_stepper_motor->step(1);
        light_level += analogRead(sensor);

    }
    m_stepper_motor->step(-CALIBRATION_ROUND_COUNT);
    digitalWrite((uint8_t) laser, LOW);
    m_sensor_threshold = (light_level / CALIBRATION_ROUND_COUNT) + 5;
#ifdef DEBUG_VIA_SERIAL
    Serial.print(F("Light sensor threshold is :"));
    Serial.println(m_sensor_threshold);
#endif
}

void LaserHarp::handle_note(int note_number) {
#ifdef DEBUG_VIA_SERIAL
    Serial.print(F("Handling note: "));
    Serial.println(note_number);
#endif
    int light_sensor_value = analogRead(Pins::sensor);
#ifdef DEBUG_VIA_SERIAL
    if (note_number % 2 == 0) {
        Serial.print(F("Light sensor value is: "));
        Serial.println(light_sensor_value);
    }
#endif
    if ((light_sensor_value > m_sensor_threshold) && (!m_notes_active_status[note_number])) // If the light sensor gets a signal, and the note is not playing:
    {
#ifdef DEBUG_VIA_SERIAL
        Serial.println(F("################################################################"));
        Serial.print(F("Turning on note: "));
        Serial.println(note_number);
#endif
        m_midiplayer->note_on(0, (byte)Notes[note_number], 127);
        m_notes_active_status[note_number] = true;
    }


    else if (light_sensor_value < m_sensor_threshold && (m_notes_active_status[note_number]))             // If the m_sensor_threshold does not get a signal:
    {
#ifdef DEBUG_VIA_SERIAL
        Serial.println(F("----------------------------------------------------------------"));
        Serial.print(F("Turning off note: "));
        Serial.println(note_number);
#endif
        m_midiplayer->note_off(0, (byte)Notes[note_number], 0x00);     // Stop playing note.
        m_notes_active_status[note_number] = false;
    }
}

void LaserHarp::handle_beam(int beam_number, int last_beam) {
#ifdef DEBUG_VIA_SERIAL
    Serial.print(F("Handling beam: "));
    Serial.println(beam_number);
    Serial.print(F("Last beam: "));
    Serial.println(last_beam);
#endif
    if (digitalRead(Pins::button) == LOW) {
#ifdef DEBUG_VIA_SERIAL
        Serial.println(F("!@#!@#!@#!@#!@##!Changing instrument."));
#endif
        m_midiplayer->set_instrument(m_instrument++);
    }
    digitalWrite((uint8_t) Pins::laser, HIGH);               // Turn on the laser for the nth beam.
    delay(m_laser_delay);
    handle_note(beam_number);
    digitalWrite((uint8_t) Pins::laser, LOW);
    delay(m_laser_delay);

    if (beam_number > last_beam) {
#ifdef DEBUG_VIA_SERIAL
        Serial.println(F("Moving one step forward"));
#endif
        m_stepper_motor->step(m_motor_step_size);
    }
    else {
#ifdef DEBUG_VIA_SERIAL
        Serial.println(F("Moving one step backward"));
#endif
        m_stepper_motor->step(-m_motor_step_size);
    }
    delay(m_beam_delay);
}


void LaserHarp::loop() {
#ifdef DEBUG_VIA_SERIAL
    Serial.println("Started going forward.");
#endif
    int last_beam = 0;

    for (int beam_number = 1; beam_number < m_beam_count; beam_number++) {
        handle_beam(beam_number, last_beam);
        last_beam = beam_number;
    }
#ifdef DEBUG_VIA_SERIAL
    Serial.println("Finished going forward.");
#endif

    last_beam = m_beam_count;
    for (int beam_number = m_beam_count; beam_number > 1; beam_number--) {
        handle_beam(beam_number, last_beam);
    }
}
