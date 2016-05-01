#include <Arduino.h>
#include "LaserHarp.h"

LaserHarp *laser_harp;
#define DEBUG_VIA_SERIAL // Uncomment this to enable serial debug.

//The setup function is called once at startup of the sketch
void setup() {
#ifdef DEBUG_VIA_SERIAL
    Serial.begin(115200); //Use serial for debugging
#endif
    laser_harp = new LaserHarp();
}

// The loop function is called in an endless loop
void loop() {
    laser_harp->loop();

}









































