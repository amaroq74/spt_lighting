#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Outputs
const unsigned int OutputPin = 12;

// Network Configuration
const char * ssid        = "parish_avnet";
const char * password    = "TechUser723";
const char * vmix_server = "172.16.24.122";
const unsigned int vmix_port = 8099;
IPAddress logAddress (172,16,24,1);
unsigned int locPort     = 8112;
unsigned int logPort     = 8111;

// Timers
const unsigned long PollPeriod = 1000; // 1 second

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define logPrintf(...) logUdp.beginPacket(logAddress,logPort); logUdp.printf(__VA_ARGS__); logUdp.endPacket();
//#define logPrintf(...) ;

// Variables
WiFiClient client;
WiFiUDP logUdp;

unsigned int lastPoll;
unsigned int x;
unsigned int currTime;
unsigned int isStreaming;
unsigned int isRecording;
String response;

char valueStr[50];
unsigned long ledTime;

// Initialize
void setup() {

   lastPoll = millis();

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
   logUdp.begin(locPort);
   delay(1000);
   logPrintf("System booted!")

   ArduinoOTA.begin();

   // Init relay
   pinMode(OutputPin,OUTPUT);
   digitalWrite(OutputPin,LOW);
}


// Service Loop
void loop() {
   ArduinoOTA.handle();

   currTime = millis();

   // Poll status
   if ( (currTime - lastPoll) > PollPeriod ) {
      isStreaming = 0;
      isRecording = 0;
      lastPoll = currTime;

      logPrintf("Polling VMIX!")

      if (client.connect(vmix_server, vmix_port)) {
         response = client.readStringUntil('\n');

         response = "";
         logPrintf("Sending: XMLTEXT vmix/streaming\n");
         client.print("XMLTEXT vmix/streaming\n");
         response = client.readStringUntil('\n');
         logPrintf("Respnse: %s\n",response.c_str());

         if ( response.indexOf("True") != -1) isStreaming = 1;

         response = "";
         logPrintf("Sending: XMLTEXT vmix/recording\n");
         client.print("XMLTEXT vmix/recording\n");
         response = client.readStringUntil('\n');
         logPrintf("Respnse: %s\n",response.c_str());

         if ( response.indexOf("True") != -1) isRecording = 1;
      }

      logPrintf("Status: streaming %i, recornding %i\n",isStreaming,isRecording);

   }

   if ( isStreaming || isRecording ) digitalWrite(OutputPin,HIGH);
   else digitalWrite(OutputPin,LOW);
}

