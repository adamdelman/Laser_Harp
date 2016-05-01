#include "LaserHarp.h"


LaserHarp::LaserHarp() {
    m_stepper_motor = new Stepper(48, A0, A2, A1, A3);
    m_stepper_motor->setSpeed(120);
    m_midiplayer = new MidiPlayer();
    m_midiplayer->enable_melodic_bank();
    m_midiplayer->set_channel_reverb(0, 5);
    m_midiplayer->vs_set_reverb(1);
    m_midiplayer->set_instrument(MIDI_GM1_INSTRUMENTS::Guitar_Harmonics);

    pinMode((uint8_t) Pins::laser, OUTPUT);            // Setup for laser.
    pinMode((uint8_t) Pins::led, OUTPUT);              // Setup for status led.
}

void LaserHarp::handle_note(int note_number) {
#ifdef DEBUG_VIA_SERIAL
    Serial.print("Handling note: ");
    Serial.println(note_number);
#endif

    int light_sensor_value = analogRead(Pins::sensor);
#ifdef DEBUG_VIA_SERIAL
    if (note_number == 0) {
        Serial.print("Light sensor value is: ");
        Serial.println(light_sensor_value);
    }
#endif
    if ((light_sensor_value > m_sensor_threshold) &&
        (!m_notes_active_status[note_number])) // If the m_sensor_threshold gets a signal, and the note is not playing:
    {
#ifdef DEBUG_VIA_SERIAL
        Serial.print("Turning on note: ");
        Serial.println(note_number);
#endif
        digitalWrite((uint8_t) Pins::led, HIGH);       // Switch on status led.
        m_midiplayer->note_on(0, Notes[note_number], 63);
        m_notes_active_status[note_number] = true;
    }


    else if (light_sensor_value < m_sensor_threshold &&
             (m_notes_active_status[note_number]))             // If the m_sensor_threshold does not get a signal:

    {
#ifdef DEBUG_VIA_SERIAL
        Serial.print("Turning off note: ");
        Serial.println(note_number);
#endif
        digitalWrite((uint8_t) Pins::led, LOW);         // Switch off the status led.
        m_midiplayer->note_off(0, Notes[note_number], 0x00);     // Stop playing note 1.
        m_notes_active_status[note_number] = false;
    }
}

void LaserHarp::handle_beam(int beam_number, int last_beam) {
#ifdef DEBUG_VIA_SERIAL
    Serial.print("Handling beam: ");
    Serial.println(beam_number);
    Serial.print("Last beam: ");
    Serial.println(last_beam);
#endif
    digitalWrite((uint8_t) Pins::laser, HIGH);               // Turn on the laser for the 1st beam.
    delay(m_laser_delay);
    handle_note(beam_number);
    digitalWrite((uint8_t) Pins::laser, LOW);

    if (beam_number > last_beam) {
#ifdef DEBUG_VIA_SERIAL
        Serial.println("Moving one step forward");
#endif
        m_stepper_motor->step(m_motor_step_size);
    }
    else {
#ifdef DEBUG_VIA_SERIAL
        Serial.println("Moving one step backward");
#endif
        m_stepper_motor->step(-m_motor_step_size);
    }
    delay(m_beam_delay);
}


void LaserHarp::loop() {
#ifdef DEBUG_VIA_SERIAL
    Serial.println("Started going forward.");
#endif
    int last_beam = -1;

    for (int beam_number = 0; beam_number < m_beam_count; beam_number++) {
        handle_beam(beam_number, last_beam);
        last_beam = beam_number;
    }
#ifdef DEBUG_VIA_SERIAL
    Serial.println("Finished going forward.");
#endif

    last_beam = 9;
    for (int beam_number = m_beam_count; beam_number > 0; beam_number--) {
        handle_beam(beam_number, last_beam);
    }
}

