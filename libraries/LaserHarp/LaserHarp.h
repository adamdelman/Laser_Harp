#ifndef MIDBURN_LASERHARP_H
#define MIDBURN_LASERHARP_H

#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <MidiPlayer.h>
#include <Stepper.h>
#include <MidiInstrument.h>

#define DEBUG_VIA_SERIAL // Uncomment this to enable serial debug.


class LaserHarp {
public:
    LaserHarp();
    void loop();
    MidiPlayer *m_midiplayer;

private:
    void handle_note(int note_number);
    void handle_beam(int beam_number, int last_beam);
    Stepper *m_stepper_motor;
    int m_stepper_motor_number_of_steps = 48;
    int m_sensor_threshold = 900;                         // Change this value to calibrate your harp's sensor
    unsigned long m_laser_delay = 50;                     // If you increase this, the laser will be brighter, but the harp will be less fluid
    unsigned long m_beam_delay = 50;
    int m_beam_count = 8;
    int m_motor_step_size = (m_stepper_motor_number_of_steps / m_beam_count)/3;

    enum  Pins {
        sensor = A4,
        laser = A5,
        led = 13,
    };
    int Notes[9] = {
            0x61,
            0x63,
            0x64,
            0x66,
            0x68,
            0x70,
            0x71,
            0x40,
            0x47,
    };
    bool m_notes_active_status[8] = {false, false, false, false, false, false, false};
};

#endif //MIDBURN_LASERHARP_H
