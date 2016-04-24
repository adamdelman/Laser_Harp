#include "MidiPlayer.h"

MidiPlayer::MidiPlayer() {
	SoftwareSerial midiSerial(2,3); // Soft TX on 3, RX not used (2 is an input anyway, for VS_DREQ)
    pinMode(VS_DREQ, INPUT);
    pinMode(VS_XCS, OUTPUT);
    pinMode(VS_XDCS, OUTPUT);
    digitalWrite(VS_XCS, HIGH); //Deselect Control
    digitalWrite(VS_XDCS, HIGH); //Deselect Data
    midiSerial.begin(31250);
    pinMode(VS_RESET, OUTPUT);


//    Initialize VS105  3 chip
    digitalWrite(VS_RESET, LOW); //Put VS1053 into hardware reset

    //Setup SPI for VS1053
    pinMode(10, OUTPUT); //Pin 10 must be set as an output for the SPI communication to work
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

    VSLoadUserCode();
    enableBank1();
}

void MidiPlayer::noteOn(byte note, byte attack_velocity) {
    talkMIDI(NOTE_ON, note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void MidiPlayer::noteOff(byte note, byte release_velocity) {
    talkMIDI(NOTE_OFF, note, release_velocity);
}

void MidiPlayer::VSWriteRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte){
    while(!digitalRead(VS_DREQ)) ; //Wait for DREQ to go high indicating IC is available
    digitalWrite(VS_XCS, LOW); //Select control

    //SCI consists of instruction byte, address byte, and 16-bit data word.
    SPI.transfer(0x02); //Write instruction
    SPI.transfer(addressbyte);
    SPI.transfer(highbyte);
    SPI.transfer(lowbyte);
    while(!digitalRead(VS_DREQ)) ; //Wait for DREQ to go high indicating command is complete
    digitalWrite(VS_XCS, HIGH); //Deselect Control
}


void MidiPlayer::VSLoadUserCode(void) {
    int i = 0;

    while (i<sizeof(sVS1053b_Realtime_MIDI_Plugin)/sizeof(sVS1053b_Realtime_MIDI_Plugin[0])) {
        unsigned short addr, n, val;
        addr = sVS1053b_Realtime_MIDI_Plugin[i++];
        n = sVS1053b_Realtime_MIDI_Plugin[i++];
        while (n--) {
            val = sVS1053b_Realtime_MIDI_Plugin[i++];
            VSWriteRegister(addr, val >> 8, val & 0xFF);
        }
    }
}



void MidiPlayer::sendMIDI(byte data)
{
    SPI.transfer(0);
    SPI.transfer(data);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void MidiPlayer::talkMIDI(byte cmd, byte data1, byte data2) {
    //
    // Wait for chip to be ready (Unlikely to be an issue with real time MIDI)
    //
    while (!digitalRead(VS_DREQ))
        ;
    digitalWrite(VS_XDCS, LOW);
    sendMIDI(cmd);
    //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes
    //(sort of: http://253.ccarh.org/handout/midiprotocol/)
    if( (cmd & 0xF0) <= 0xB0 || (cmd & 0xF0) >= 0xE0) {
        sendMIDI(data1);
        sendMIDI(data2);
    } else {
        sendMIDI(data1);
    }

    digitalWrite(VS_XDCS, HIGH);
}

void MidiPlayer::setInstrument(int midiInstrument){
    talkMIDI(0xC0, (byte)midiInstrument, 0); //Set instrument number. 0xC0 is a 1 data byte command

}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15


void MidiPlayer::demo() {
    delay(1000);

    talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)

    //Demo Basic MIDI instruments, GM1
    //=================================================================
    Serial.println("Basic Instruments");
    talkMIDI(0xB0, 0, 0x00); //Default bank GM1

    //Change to different instrument
    for(int instrument = 60 ; instrument < 127 ; instrument++) {

        Serial.print(" Instrument: ");
        Serial.println(instrument, DEC);

        talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command

        //Play notes from F#-0 (30) to F#-5 (90):
        for (int note = 30 ; note < 40 ; note++) {
            Serial.print("N:");
            Serial.println(note, DEC);

            //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
            noteOn(note, 127);
            delay(200);

            //Turn off the note with a given off/release velocity
            noteOff(note, 127);
            delay(50);
        }

        delay(100); //Delay between instruments
    }
    //=================================================================


}

void MidiPlayer::demo2(){
    //Demo GM2 / Fancy sounds
    //=================================================================
    Serial.println("Demo Fancy Sounds");
    talkMIDI(0xB0, 0, 0x78); //Bank select drums
    //For this bank 0x78, the instrument does not matter, only the note
    for(int instrument = 30 ; instrument < 31 ; instrument++) {
        Serial.print(" Instrument: ");
        Serial.println(instrument, DEC);
        talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command
        //Play fancy sounds from 'High Q' to 'Open Surdo [EXC 6]'
        for (int note = 27 ; note < 87 ; note++) {
            Serial.print("N:");
            Serial.println(note, DEC);

            //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
            noteOn(note, 127);
            delay(50);
            //Turn off the note with a given off/release velocity
            noteOff(note, 127);
            delay(50);
        }
        delay(100); //Delay between instruments
    }
}

void MidiPlayer::enableBank1() {
    talkMIDI(0xB0, 0, 0x00); //Default bank GM1
}


void MidiPlayer::enableMelodicBank(){
    talkMIDI(0xB0, 0, 0x79);
}

