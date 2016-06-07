#include "MidiPlayer.h"

MidiPlayer::MidiPlayer() {
    SoftwareSerial midiSerial(2, 3); // Soft TX on 3, RX not used (2 is an input anyway, for VS_DREQ)
    pinMode(VS_DREQ, INPUT);
    pinMode(VS_XCS, OUTPUT);
    pinMode(VS_XDCS, OUTPUT);
    digitalWrite(VS_XCS, HIGH); //Deselect Control
    digitalWrite(VS_XDCS, HIGH); //Deselect Data
    midiSerial.begin(31250);
    pinMode(VS_RESET, OUTPUT);


//    Initialize VS1053 chip
    digitalWrite(VS_RESET, LOW); //Put VS1053 into hardware reset

    //Setup SPI for VS1053
    pinMode(SPI_PIN, OUTPUT); //Pin 10 must be set as an output for the SPI communication to work
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

    //From page 12 of datasheet, max SCI reads are CLKI/7. Input clock is 12.288MHz.
    //Internal clock multiplier is 1.0x after power up.
    //Therefore, max SPI speed is 1.75MHz. We will use 1MHz to be safe.
    SPI.setClockDivider(SPI_CLOCK_DIV16); //Set SPI bus speed to 1MHz (16MHz / 16 = 1MHz)
    SPI.transfer(0xFF); //Throw a dummy byte at the bus

    delayMicroseconds(1);
    digitalWrite(VS_RESET, HIGH); //Bring up VS1053

    vs_load_user_code();
    vs_write_register(VS_SCI_CLOCKF_REGISTER,  0x80, 0x00); //Set multiplier to 3.5x
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void MidiPlayer::note_on(byte channel, byte note, byte attack_velocity) {
    send_midi_message(MIDI_NOTE_ON_MESSAGE | channel, note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void MidiPlayer::note_off(byte channel, byte note, byte release_velocity) {
    send_midi_message(MIDI_NOTE_OFF_MESSAGE | channel, note, release_velocity);
}

/**
   Set the MIDI channel volume.
   @param[in]     volume The volume to set. Can be 0-127.
 */
void MidiPlayer::set_channel_volume(byte channel, int volume) {
    send_midi_message(MIDI_PROGRAM_CHANGE_MESSAGE | channel, MIDI_NOTE_VOLUME_LEVEL, volume);
}


/**
   Set the MIDI channel reverberation level.
   @param[in]     reverb_level The reverb level to set. Can be 0-127.
 */
void MidiPlayer::set_channel_reverb(byte channel, int reverb_level) {
    send_midi_message(MIDI_PROGRAM_CHANGE_MESSAGE | channel, MIDI_REVERB_LEVEL, reverb_level);
}

void MidiPlayer::demo() {
    delay(1000);
    enable_gm1_bank();
    set_channel_volume(0,120); //set channel volume to near max (127)

    //Demo Basic MIDI instruments, GM1
    //=================================================================
#ifdef DEBUG_VIA_SERIAL
    Serial.println(F("Basic Instruments Demo"));
    Serial.println(F("######################"));
#endif
    //Change to different instrument
    for (byte instrument = (byte) MIDI_GM1_INSTRUMENTS::Guitar_Fret_Noise;
         instrument < (byte) MIDI_GM1_INSTRUMENTS::Bird_Tweet; instrument++) {
#ifdef DEBUG_VIA_SERIAL
        Serial.print(F("Instrument: #"));
        Serial.println(instrument, DEC);
#endif
        set_instrument(instrument); //Set instrument number.

        //Play notes from F#-0 (30) to F#-5 (90):
        for (byte note = 30; note < 35; note++) {
#ifdef DEBUG_VIA_SERIAL
            Serial.print(F("Note: #"));
            Serial.println(note, DEC);
#endif

            //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
            note_on(0, note, 127);
            delay(200);

            //Turn off the note with a given off/release velocity
            note_off(0, note, 127);
            delay(50);
        }

        delay(100); //Delay between instruments
    }
    //=================================================================


}

void MidiPlayer::demo2() {
    delay(1000);

    //Demo GM2 / Fancy sounds
    //=================================================================
#ifdef DEBUG_VIA_SERIAL
    Serial.println(F("Fancy Sounds Demo"));
    Serial.println(F("#################"));
#endif
    enable_melodic_bank();
    set_channel_reverb(0,127);
    vs_set_reverb(13);
    for (byte instrument = 49;
         instrument < 52; instrument++) {
#ifdef DEBUG_VIA_SERIAL
        Serial.print(F("Instrument: #"));
        Serial.println(instrument, DEC);
#endif
        set_instrument(instrument); //Set instrument number.
        //Play fancy sounds from 'High Q' to 'Open Surdo [EXC 6]'
        for (byte note = 50; note < 53; note++) {
#ifdef DEBUG_VIA_SERIAL
            Serial.print(F("Note: #"));
            Serial.println(note, DEC);
#endif
            //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
            note_on(0,note, 127);
            delay(1000);
            //Turn off the note with a given off/release velocity
            note_off(0,note, 127);
            delay(50);
        }
        delay(100); //Delay between instruments
    }
}

/**
   Enable the GM 1 MIDI bank (This is the default if no bank is set).
 */
void MidiPlayer::enable_gm1_bank() {
    send_midi_message(MIDI_PARAMETER_MESSAGE, MIDI_BANK_SELECT, MIDI_BANK_GM1);
}

/**
   Enable the precussion MIDI bank.
 */
void MidiPlayer::enable_percussion_bank() {
    send_midi_message(MIDI_PARAMETER_MESSAGE, MIDI_BANK_SELECT, MIDI_BANK_PERCUSSION_1);
}

/**
   Enable the melodic MIDI bank.
 */
void MidiPlayer::enable_melodic_bank() {
    send_midi_message(MIDI_PARAMETER_MESSAGE, MIDI_BANK_SELECT, MIDI_BANK_MELODIC_GM2);
}


/**
   Set the active MIDI instrument.
   @param[in]     midi_instrument The number of the MIDI instrument (See MidiInstrument.h for a complete list).
 */
void MidiPlayer::set_instrument(int midi_instrument) {
    send_midi_message(MIDI_PROGRAM_CHANGE_MESSAGE, (byte) midi_instrument);

}

void MidiPlayer::send_midi(byte data) {
    SPI.transfer(0);
    SPI.transfer(data);
}

//Read the 16-bit value of a VS10xx register
unsigned int MidiPlayer::vs_read_register(unsigned int address_byte) {
    vs_wait_for_command_ready();
    digitalWrite(VS_XCS, LOW); //Select control
    //SCI consists of instruction byte, address byte, and 16-bit data word.
    SPI.transfer(0x03);  //Read instruction
    SPI.transfer(address_byte);

    char response1, response2;
    response1 = SPI.transfer(0xFF); //Read the first byte
    vs_wait_for_command_ready();
    response2 = SPI.transfer(0xFF); //Read the second byte
    vs_wait_for_command_ready();

    digitalWrite(VS_XCS, HIGH); //Deselect Control

    int result_value = response1 << 8;
    result_value |= response2;
    return result_value;
}

void MidiPlayer::vs_write_register(unsigned int address_byte, unsigned int high_byte, unsigned int low_byte) {
    vs_wait_for_command_ready();
    digitalWrite(VS_XCS, LOW); //Select control

    //SCI consists of instruction byte, address byte, and 16-bit data word.
    SPI.transfer(0x02); //Write instruction
    SPI.transfer(address_byte);
    SPI.transfer(high_byte);
    SPI.transfer(low_byte);
    vs_wait_for_command_ready();
    digitalWrite(VS_XCS, HIGH); //Deselect Control
}

void MidiPlayer::vs_set_volume(unsigned int left_channel_volume, unsigned int right_channel_volume) {
    vs_write_register(VS_SCI_VOL_REGISTER, left_channel_volume, right_channel_volume);
}


void MidiPlayer::vs_set_reverb(int reverb_level) {
    unsigned int current_config;
    vs_write_register(VS_SCI_WRAMADDR_REGISTER, 0, 0x1e03); // Register Parametric.config1 address
    current_config = vs_read_register(VS_SCI_WRAM_REGISTER); // Read config1
    current_config = (current_config & ~0xf) |
                     reverb_level;    // Set reverb to 1 = off (see VS1053 Datasheet Chapter 9.11.4: F("Midi"))
    vs_write_register(VS_SCI_WRAMADDR_REGISTER, 0, 0x1e03);
    vs_write_register(VS_SCI_WRAM_REGISTER, 0,
                      current_config); // Write the new value for config1. Now Midi echo is disabled.
}

void MidiPlayer::vs_wait_for_command_ready() {
    while (!vs_is_ready_for_command());
}

bool MidiPlayer::vs_is_ready_for_command() {
    return digitalRead(VS_DREQ);
}

void MidiPlayer::vs_load_user_code(void) {
    int i = 0;

    while (i < sizeof(sVS1053b_Realtime_MIDI_Plugin) / sizeof(sVS1053b_Realtime_MIDI_Plugin[0])) {
        unsigned int addr, n, val;
        addr = sVS1053b_Realtime_MIDI_Plugin[i++];
        n = sVS1053b_Realtime_MIDI_Plugin[i++];
        while (n--) {
            val = sVS1053b_Realtime_MIDI_Plugin[i++];
            vs_write_register(addr, val >> 8, val & 0xFF);
        }
    }
}


//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void MidiPlayer::send_midi_message(byte cmd, byte data1, byte data2) {
    vs_wait_for_command_ready();
    digitalWrite(VS_XDCS, LOW);
    send_midi(cmd);
    send_midi(data1);
    send_midi(data2);
    digitalWrite(VS_XDCS, HIGH);
}


void MidiPlayer::send_midi_message(byte cmd, byte data) {
    vs_wait_for_command_ready();
    digitalWrite(VS_XDCS, LOW);
    send_midi(cmd);
    send_midi(data);
    digitalWrite(VS_XDCS, HIGH);
}

void MidiPlayer::print_details()
{
    Serial.println(F("VS1053 Configuration"));
    Serial.print(F("MODE "));
    Serial.println(vs_read_register(0));
    Serial.print(F("STATUS "));
    Serial.println(vs_read_register(1));
    Serial.print(F("BASS "));
    Serial.println(vs_read_register(2));
    Serial.print(F("CLOCKF "));
    Serial.println(vs_read_register(3));
    Serial.print(F("DECODE_TIME "));
    Serial.println(vs_read_register(4));
    Serial.print(F("AUDATA "));
    Serial.println(vs_read_register(5));
    Serial.print(F("WRAM "));
    Serial.println(vs_read_register(6));
    Serial.print(F("WRAMADDR "));
    Serial.println(vs_read_register(7));
    Serial.print(F("HDAT0 "));
    Serial.println(vs_read_register(8));
    Serial.print(F("HDAT1 "));
    Serial.println(vs_read_register(9));
    Serial.print(F("AIADDR "));
    Serial.println(vs_read_register(10));
    Serial.print(F("VOL "));
    Serial.println(vs_read_register(11));
    Serial.print(F("AICTRL0 "));
    Serial.println(vs_read_register(12));
    Serial.print(F("AICTRL1 "));
    Serial.println(vs_read_register(13));
    Serial.print(F("AICTRL2 "));
    Serial.println(vs_read_register(14));
    Serial.print(F("AICTRL3 "));
    Serial.println(vs_read_register(15));
}