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
    int m_sensor_threshold = 1000;
    unsigned long m_laser_delay = 25;   // If you increase this, the laser will be brighter, but the harp will be less fluid
    unsigned long m_beam_delay = 0;
    int m_beam_count = 5;
    int m_motor_step_size = 1;
    int m_instrument;

    enum Pins {
        sensor = A4,
        laser = A5,
        button = 5
    };
    int Notes[9] = {
            0x60,
            0x62,
            0x64,
            0x65,
            0x67,
            0x69,
            0x71,
            0x72,
            0x74,
    };
//    int Notes[9] = {
//            0x60,
//            0x61,
//            0x62,
//            0x63,
//            0x64,
//            0x65,
//            0x66,
//            0x67,
//            0x68,
//    };
    bool m_notes_active_status[9] = {false, false, false, false, false, false, false, false, false};

    void calibrate_light_sensor();
};

#endif //MIDBURN_LASERHARP_H
