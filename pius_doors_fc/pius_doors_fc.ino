#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

// Configuration
const char * ssid        = "parish_avnet";
const char * password    = "TechUser723";
const char * mqtt_server = "saint.pius.org";

//const char * ssid        = "amaroq";
//const char * password    = "1er4idnfu345os3o283";
//const char * mqtt_server = "aliska.amaroq.net";

// Timers
const unsigned long DigitalPeriod  = 300000; // 5 minutes
const unsigned long DebouncePeriod = 250;    // 0.25 second

// Inputs
const unsigned int InputCount = 2;
const char * InputStatTopic[] = {"stat/fc/door1",
                                 "stat/fc/door2"};
unsigned int InputPin[]       = {4, 5};

const char * WifiTopic = "stat/fc/wifi";

// Variables
unsigned int lastDigital = millis();
unsigned int x;
unsigned int currTime;

char valueStr[50];

unsigned int inputLevel[InputCount];
unsigned int inputRaw[InputCount];
unsigned int inputTime[InputCount];

WiFiClient espClient;
PubSubClient client(espClient);

unsigned int tmp;

int wifiPct() {
   long rssi = WiFi.RSSI();

   if ( rssi >= -50 ) return 100;
   else if ( rssi <= -100 ) return 0;
   else return (int(2 * (rssi + 100)));
}

// Initialize
void setup() {

   lastDigital = millis();

   // Start and connect to WIFI
   WiFi.mode(WIFI_STA);
   WiFi.setSleepMode(WIFI_NONE_SLEEP);
   WiFi.begin(ssid, password);

   while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      delay(5000);
      ESP.restart();
   }
   delay(1000);

   // Firmware download start callback
   ArduinoOTA.onStart([]() { });

   // Firmware download done callback
   ArduinoOTA.onEnd([]() { });

   // Firmware download progress
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { });

   ArduinoOTA.onError([](ota_error_t error) { });

   ArduinoOTA.begin();

   client.setServer(mqtt_server, 1883);

   // Init inputs
   for (x=0; x < InputCount; x++) {
      pinMode(InputPin[x],INPUT);
      inputLevel[x] = 0;
      inputRaw[x]   = 0;
      inputTime[x]  = millis();
   }
}


// MQTT Connect
void reconnect() {

   // Loop until we're reconnected
   while (!client.connected()) {

      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);

      // Attempt to connect
      if (client.connect(clientId.c_str())) {
      } else {
         delay(5000);
      }
   }
   delay(5000);
}


// Service Loop
void loop() {
   ArduinoOTA.handle();

   if (!client.connected()) reconnect();
   client.loop();

   currTime = millis();

   // Read digital input states
   for (x=0; x < InputCount; x++) {
      if ( digitalRead(InputPin[x]) == 1 ) tmp = 1;
      else tmp = 0;

      // Digital Input Debounce
      if ( tmp != inputRaw[x]) inputTime[x] = currTime;

      // Input is stable
      if ( (currTime - inputTime[x]) > DebouncePeriod ) {

         // State changed
         if ( inputRaw[x] != inputLevel[x] ) {

            // Update state
            inputLevel[x] = inputRaw[x];

            if (inputLevel[x] == 1 )  {
               client.publish(InputStatTopic[x],"Open");
            }
            else {
               client.publish(InputStatTopic[x],"Closed");
            }
            delay(10);
         }
      }

      inputRaw[x] = tmp;
   }

   // Refresh digital values
   if ( (currTime - lastDigital) > DigitalPeriod) {

      for (x=0; x < InputCount; x++) {
         if (inputLevel[x] == 1 )
            client.publish(InputStatTopic[x],"Open");
         else
            client.publish(InputStatTopic[x],"Closed");
         delay(10);
      }

      lastDigital = currTime;
      sprintf(valueStr,"%i",wifiPct());
      client.publish(WifiTopic,valueStr);
   }
}

