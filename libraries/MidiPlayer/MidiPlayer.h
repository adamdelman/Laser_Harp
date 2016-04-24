#ifndef MIDBURN_MIDIPLAYER_H
#define MIDBURN_MIDIPLAYER_H

#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "MidiInstrument.h"

//PIN definitions
#define VS_DREQ   2 // Data Request Pin: Player asks for more data
#define VS_XCS    6 // Control Chip Select Pin (for accessing SPI Control/Status registers)
#define VS_XDCS   7 // Data Chip Select / BSYNC Pin
#define VS_RESET  8 //Reset is active low

const unsigned short sVS1053b_Realtime_MIDI_Plugin[28] = { /* Compressed plugin */
0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
0x0200, 0x000a, 0x0001, 0x0050, };

static const int NOTE_OFF = 0x80;

static const int NOTE_ON = 0x90;

class MidiPlayer {
	bool debug_enabled;
public:
	MidiPlayer();
	void demo();
	void demo2();
	void noteOn(byte note, byte attack_velocity);
	void noteOff(byte note, byte release_velocity);
    void enableBank1();
	void enableMelodicBank();
	void setInstrument(int midiInstrument);
private:
	void VSWriteRegister(unsigned char addressbyte, unsigned char highbyte,
			unsigned char lowbyte);
	void talkMIDI(byte cmd, byte data1, byte data2);
	void VSLoadUserCode(void);
	void sendMIDI(byte data);



};

#endif //MIDBURN_MIDIPLAYER_H
