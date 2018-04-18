

#include "Conceptinetics.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define DMX_SLAVE_CHANNELS 4
// Chan 0 = rel0
// Chan 1 = rel1
// Chan 2 = rel2
// Chan 3 = rel3

// Configure a DMX slave controller
DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS );

const int relPin[4] = {8,9,10,11};

const int dmxBase   = 420;

// the setup routine runs once when you press reset:
void setup() {             
   uint8_t x;
  
   // Enable DMX slave interface and start recording
   // DMX data
   dmx_slave.enable ();  
  
   // Set start address to 1, this is also the default setting
   // You can change this address at any time during the program
   dmx_slave.setStartAddress (dmxBase);

   for (x=0; x < 4; x++) pinMode(relPin[x],OUTPUT);
}

void loop() {
   uint8_t x;

   for (x=0; x <4; x++) {
      if ( dmx_slave.getChannelValue(x+1) > 127 ) 
         digitalWrite(relPin[x],HIGH);
      else
         digitalWrite(relPin[x],LOW);
   }
   delay(100);
}

