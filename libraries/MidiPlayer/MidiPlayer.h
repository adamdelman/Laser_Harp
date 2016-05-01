#ifndef MIDBURN_MIDIPLAYER_H
#define MIDBURN_MIDIPLAYER_H

#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "MidiInstrument.h"

#define DEBUG_VIA_SERIAL // Uncomment this to enable serial debug.

//PIN definitions
#define VS_DREQ   2 // Data Request Pin: Player asks for more data
#define VS_XCS    6 // Control Chip Select Pin (for accessing SPI Control/Status registers)
#define VS_XDCS   7 // Data Chip Select / BSYNC Pin
#define VS_RESET  8 //Reset is active low
#define SPI_PIN 10

//VS10xx SCI Registers
#define VS_SCI_MODE_REGISTER 0x00 //Mode control
#define VS_SCI_STATUS_REGISTER 0x01 //Status of VS1053b
#define VS_SCI_BASS_REGISTER 0x02 //Built-in bass/treble control
#define VS_SCI_CLOCKF_REGISTER 0x03 //Clock freq + multiplier
#define VS_SCI_DECODE_TIME_REGISTER 0x04 // Decode time in seconds
#define VS_SCI_AUDATA_REGISTER 0x05 //Misc. audio data
#define VS_SCI_WRAM_REGISTER 0x06 //RAM write/read
#define VS_SCI_WRAMADDR_REGISTER 0x07 //Base address for RAM write/read
#define VS_SCI_HDAT0_REGISTER 0x08 //Stream header data 0
#define VS_SCI_HDAT1_REGISTER 0x09 //Stream header data 1
#define VS_SCI_AIADDR_REGISTER 0x0A //Start address of application
#define VS_SCI_VOL_REGISTER 0x0B //Volume control
#define VS_SCI_AICTRL0_REGISTER 0x0C //Application control register 0
#define VS_SCI_AICTRL1_REGISTER 0x0D //Application control register 1
#define VS_SCI_AICTRL2_REGISTER 0x0E //Application control register 2
#define VS_SCI_AICTRL3_REGISTER 0x0F //Application control register 3

//MIDI message types
#define MIDI_PARAMETER_MESSAGE 0xb0
#define MIDI_PROGRAM_CHANGE_MESSAGE 0xc0
#define MIDI_NOTE_OFF_MESSAGE 0x80
#define MIDI_NOTE_ON_MESSAGE 0x90

// Midi parameters
#define MIDI_BANK_SELECT 0x00
#define MIDI_RPN_MSB 0x06// 0 = bend range, 2 = coarse tune
#define MIDI_NOTE_VOLUME_LEVEL 0x07 // from 0 to 127
#define MIDI_REVERB_LEVEL 0x5b // channel reverb level
#define MIDI_PAN_CONTROL 0x0a
#define MIDI_EXPRESSION_CONTROL 0x0b // expression (changes volume)
#define MIDI_EFFECT_CONTROL 0x0c// effect control 1 (sets global reverb decay)
#define MIDI_RPN_LSB 0x26// 0 = bend range
#define MIDI_MUTE 0x78
//0x40: hold1
//
//0x42: sustenuto
//
//0x62,0x63,0x64,0x65: NRPN and RPN selects
//0x79: reset all controllers
//0x7b, 0x7c, 0x7d all notes off

// Midi Banks
#define MIDI_BANK_GM1 0x00
#define MIDI_BANK_PERCUSSION_1 0x78
#define MIDI_BANK_PERCUSSION_2 0x7f
#define MIDI_BANK_MELODIC_GM2 0x79


#define SM_EARSPEAKER_LO (1<<4)
#define SM_EARSPEAKER_HI (1<<7)


const unsigned short sVS1053b_Realtime_MIDI_Plugin[28] = { /* Compressed plugin */
        0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
        0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
        0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
        0x0200, 0x000a, 0x0001, 0x0050,};


class MidiPlayer {
public:
    MidiPlayer();

    void demo();

    void demo2();

    void set_channel_volume(byte channel, int volume);

    void set_channel_reverb(byte channel, int reverb_level);

    void note_on(byte channel, byte note, byte attack_velocity);

    void note_off(byte channel, byte note, byte release_velocity);

    void enable_gm1_bank();

    void enable_percussion_bank();

    void enable_melodic_bank();

    void set_instrument(int midi_instrument);
    void print_details();
    void vs_set_volume(unsigned int left_channel_volume, unsigned int right_channel_volume);

    void vs_set_reverb(int reverb_level);
private:
    void vs_wait_for_command_ready();

    bool vs_is_ready_for_command();


    void vs_write_register(unsigned int address_byte, unsigned int high_byte,
                           unsigned int low_byte);

    unsigned int vs_read_register(unsigned int address_byte);

    void send_midi_message(byte cmd, byte data1, byte data2);
    void send_midi_message(byte cmd, byte data);


    void vs_load_user_code(void);

    void send_midi(byte data);


};

#endif //MIDBURN_MIDIPLAYER_H
