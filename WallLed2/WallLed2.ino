#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>
#include <ESPDMX.h>

DMXESPSerial dmx;

// Configuration
//const char * ssid        = "gydev";
//const char * password    = "GyDevice3389";
const char * ssid        = "amaroq";
const char * password    = "1er4idnfu345os3o283";

// Chan 0 = max
// Chan 1 = red
// Chan 2 = green
// Chan 3 = blue
// Chan 4 = mode

const int ledPin1   = 4;
const int ledPin2   = 5;
const int dmxBase   = 410;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(300, ledPin1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(300, ledPin2, NEO_GRB + NEO_KHZ800);

WiFiServer server(80);

unsigned int currWwwTime;
unsigned int prevWwwTime;
unsigned int x;
String header;
uint8_t level;
uint8_t red;
uint8_t green;
uint8_t blue;
uint8_t mode;

const unsigned long TimeoutPeriod = 2000;  // 2 seconds

int wifiPct() {
   long rssi = WiFi.RSSI();

   if ( rssi >= -50 ) return 100;
   else if ( rssi <= -100 ) return 0;
   else return (int(2 * (rssi + 100)));
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t pos, uint8_t level) {
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

   if ( r > level ) r = level;
   if ( g > level ) g = level;
   if ( b > level ) b = level;
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

void setRainbow(uint8_t level, uint16_t shift, uint8_t gap) {
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
      else setPixel(pos,Wheel(wpos,level));
   }
}

void wwwClient() {
   WiFiClient client = server.available();

   if (client) {
      String currentLine = "";                // make a String to hold incoming data from the client
      currWwwTime = millis();
      prevWwwTime = currWwwTime;

      while (client.connected() && currWwwTime - prevWwwTime <= TimeoutPeriod) { // loop while the client's connected
         currWwwTime = millis();

         if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();             // read a byte, then
            header += c;

            if (c == '\n') {                    // if the byte is a newline character
               // if the current line is blank, you got two newline characters in a row.
               // that's the end of the client HTTP request, so send a response:
               if (currentLine.length() == 0) {

                  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                  // and a content-type so the client knows what's coming, then a blank line:
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-type:text/html");
                  client.println("Connection: close");
                  client.println();

                  // Display the HTML web page
                  client.println("<!DOCTYPE html><html>");
                  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                  client.println("<link rel=\"icon\" href=\"data:,\">");

                  // CSS to style the on/off buttons
                  // Feel free to change the background-color and font-size attributes to fit your preferences
                  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                  client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
                  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                  client.println(".button2 {background-color: #77878A;}</style></head>");

                  // Web Page Heading
                  client.println("<body><center>");
                  client.println("<h1>Wall 1 Light Control</h1>");

                  client.print("<p>DMX Values: <p>");
                  client.print("Level: ");
                  client.print(level);
                  client.println("<br>\n");
                  client.print("Red: ");
                  client.print(red);
                  client.println("<br>\n");
                  client.print("Green: ");
                  client.print(green);
                  client.println("<br>\n");
                  client.print("Blue: ");
                  client.print(blue);
                  client.println("<br>\n");
                  client.print("Mode: ");
                  client.print(mode);
                  client.println("<br>\n");

                  client.print("<p>WIFI Strength: ");
                  client.print(wifiPct());
                  client.println(" %<br>");

                  client.println("</center>");
                  client.println("</body></html>");

                  // The HTTP response ends with another blank line
                  client.println();

                  // Break out of the while loop
                  break;

               } else { // if you got a newline, then clear currentLine
                  currentLine = "";
               }
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
               currentLine += c;      // add it to the end of the currentLine
            }
         }
      }

      // Clear the header variable
      header = "";

      // Close the connection
      client.stop();
   }
}


// Initialize
void setup() {

   // Start and connect to WIFI
   WiFi.mode(WIFI_STA);
   WiFi.setSleepMode(WIFI_NONE_SLEEP);
   WiFi.begin(ssid, password);

   while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      delay(5000);
      ESP.restart();
   }
   delay(1000);

   // Start logging
   delay(1000);

   // Firmware download start callback
   ArduinoOTA.onStart([]() {
      if (ArduinoOTA.getCommand() == U_FLASH) { }
      else { // U_SPIFFS
      }
   });

   // Firmware download done callback
   ArduinoOTA.onEnd([]() { });

   // Firmware download progress
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { });

   ArduinoOTA.onError([](ota_error_t error) {
      //if      (error == OTA_AUTH_ERROR)    {}
      //else if (error == OTA_BEGIN_ERROR)   {}
      //else if (error == OTA_CONNECT_ERROR) {}
      //else if (error == OTA_RECEIVE_ERROR) {}
      //else if (error == OTA_END_ERROR)     {}
   });

   ArduinoOTA.begin();
   server.begin();

   Serial.begin(9600);
   dmx.init();               // initialization

   strip1.begin();
   strip1.show();
   strip2.begin();
   strip2.show();
}


// Service Loop
void loop() {
   ArduinoOTA.handle();

   dmx.update();

   level = dmx.read(dmxBase + 0);
   red   = dmx.read(dmxBase + 1);
   green = dmx.read(dmxBase + 2);
   blue  = dmx.read(dmxBase + 3);
   mode  = dmx.read(dmxBase + 4);

   if ( red   > level ) red   = level;
   if ( green > level ) green = level;
   if ( blue  > level ) blue  = level;

   setAllPixels(strip1.Color(red,green,blue),mode);
   //if ( mode < 10 ) setAllPixels(strip1.Color(red,green,blue),mode);
   //else if ( mode < 100 ) setRainbow(max,0,0);
   //else setRainbow(max,(mode-100)*4,0);

   strip1.show();
   strip2.show();
   wwwClient();
}

