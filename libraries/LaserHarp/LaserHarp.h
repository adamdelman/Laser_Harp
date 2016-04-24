#ifndef MIDBURN_LASERHARP_H
#define MIDBURN_LASERHARP_H

#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "MidiPlayer.h"
#include <Stepper.h>


class LaserHarp {
public:
    LaserHarp(MidiPlayer *midiplayer, Stepper *stepper_motor);
    void loop();

private:
    void handle_note(int note_number);
    void handle_beam(int beam_number, int last_beam);
    MidiPlayer *m_midiplayer;
    Stepper *m_stepper_motor;
    int m_stepper_motor_number_of_steps = 48;
    int sensor_threshold = 8;                         // Change this value to calibrate your harp's sensor
    int laser_delay = 5;                     // If you increase this, the laser will be brighter, but the harp will be less fluid
    const int m_beam_count = 8;
    const int m_motor_step_size = (m_stepper_motor_number_of_steps / m_beam_count)/3;

    enum Pins {
        laser = 12,
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
