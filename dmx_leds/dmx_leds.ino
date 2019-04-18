

#include "Conceptinetics.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define DMX_SLAVE_CHANNELS 6
// Chan 0 = max
// Chan 1 = red
// Chan 2 = green
// Chan 3 = blue
// Chan 4 = mode

// Configure a DMX slave controller
DMX_Slave dmx_slave ( DMX_SLAVE_CHANNELS );

const int ledPin1   = 6;
const int ledPin2   = 7;
const int dmxBase   = 410;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(300, ledPin1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(300, ledPin2, NEO_GRB + NEO_KHZ800);

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t pos, uint8_t max) {
   uint8_t r,g,b;

   pos = 255 - pos;
   if(pos < 85) {
      r = 255-(pos*3);
      g = 0;
      b = (pos * 3);
   }
   else if(pos < 170) {
      pos -= 85;
      r = 0;
      g = (pos*3);
      b = 255-(pos*3);
   }
   else {
      pos -= 170;
      r = pos*3;
      g = 255 - (pos*3);
      b = 0;
   }

   if ( r > max ) r = max;
   if ( g > max ) g = max;
   if ( b > max ) b = max;
  return(strip1.Color(r,g,b));
}

// Set an led color across both strips left to right. The first strip order
// is reversed
void setPixel(uint16_t pixel, uint32_t color) {
   if (pixel < 300) strip1.setPixelColor(299-pixel,color);
   else strip2.setPixelColor(pixel-300,color);
}

void setAllPixels(uint32_t color, uint8_t gap) {
   uint16_t x;
   for ( x=0; x < 300; x++ ) {
      //if ( (300-x) >= gap ) strip1.setPixelColor(x,color);
      //else strip1.setPixelColor(x,0);
      //if ( (300-x) >= gap ) strip2.setPixelColor(x,color);
      //else strip2.setPixelColor(x,0);
      if ( x >= gap ) strip1.setPixelColor(x,color);
      else strip1.setPixelColor(x,0);
      if ( x >= gap ) strip2.setPixelColor(x,color);
      else strip2.setPixelColor(x,0);
   }
}

void setRainbow(uint8_t max, uint16_t shift, uint8_t gap) {
   uint16_t pos;
   uint8_t  wpos;
   uint16_t gmin;
   uint16_t gmax;
   uint16_t x;

   gmin = 300 - gap;
   gmax = 300 + gap;

   for (x=0; x < 600; x++) {
      pos = (x + shift) % 600;
      wpos = int(((float)x / 599.0) * 255.0);

      if ( (gap > 0) && ( (x > gmin) && (x < gmax)) ) setPixel(pos,0);
      else setPixel(pos,Wheel(wpos,max));
   }
}

// the setup routine runs once when you press reset:
void setup() {             
  
   // Enable DMX slave interface and start recording
   // DMX data
   dmx_slave.enable ();  
  
   // Set start address to 1, this is also the default setting
   // You can change this address at any time during the program
   dmx_slave.setStartAddress (dmxBase);
  
   strip1.begin();
   strip1.show();
   strip2.begin();
   strip2.show();
}

void loop() {
   uint8_t max;
   uint8_t red;
   uint8_t green;
   uint8_t blue;
   uint8_t mode;

   max   = dmx_slave.getChannelValue(1);
   red   = dmx_slave.getChannelValue(2);
   green = dmx_slave.getChannelValue(3);
   blue  = dmx_slave.getChannelValue(4);
   mode  = dmx_slave.getChannelValue(5);

   if ( red   > max ) red   = max;
   if ( green > max ) green = max;
   if ( blue  > max ) blue  = max;

   setAllPixels(strip1.Color(red,green,blue),mode);
   //if ( mode < 10 ) setAllPixels(strip1.Color(red,green,blue),mode);
   //else if ( mode < 100 ) setRainbow(max,0,0);
   //else setRainbow(max,(mode-100)*4,0);

   strip1.show();
   strip2.show();
   delay(100);
}


