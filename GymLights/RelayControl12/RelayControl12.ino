#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

// Configuration
const char * ssid        = "parish_avnet";
const char * password    = "TechUser723";
unsigned int locPort     = 8112;
unsigned int logPort     = 8111;
IPAddress    logAddress (172,16,20,1);

// Timers
const unsigned long MsgTxPeriod   = 1000;  // 1 second
const unsigned long DigitalPeriod = 60000; // 1 minute
const unsigned long PulsePeriod   = 1500;  // 2 seconds
const unsigned long TimeoutPeriod = 2000;  // 2 seconds

// Outputs
const unsigned int OutputCount = 6;

// Output Map
const unsigned int OutputOn[OutputCount]  = {6,4,2,0,10,8};
const unsigned int OutputOff[OutputCount] = {7,5,3,1,11,9};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Variables
unsigned int lastMsgTx   = millis();
unsigned int lastDigital = millis();
unsigned int x;
unsigned int currTime;
unsigned int currWwwTime;
unsigned int prevWwwTime;

//WiFiUDP logUdp;
WiFiServer server(80);

unsigned int rxCount;
unsigned int onChan;
unsigned int offChan;

char txBuffer[50];
char rxBuffer[50];
char tmpStr[100];
char mark[10];
int  tmp;
char c;

String header;

char  valueStr[50];

unsigned int outputState[OutputCount*2];
unsigned int outputTime[OutputCount*2];
unsigned int statusState[OutputCount*2];

// Macro for logging
//#define logPrintf(...) logUdp.beginPacket(logAddress,logPort); logUdp.printf(__VA_ARGS__); logUdp.endPacket();
#define logPrintf(...) ;

int wifiPct() {
   long rssi = WiFi.RSSI();

   if ( rssi >= -50 ) return 100;
   else if ( rssi <= -100 ) return 0;
   else return (int(2 * (rssi + 100)));
}


// Send message to arduino
void sendMsg() {

    sprintf(txBuffer,"STATE %i %i %i %i %i %i %i %i %i %i %i %i\n",
                     outputState[0], outputState[1], outputState[2],  outputState[3],
                     outputState[4], outputState[5], outputState[6],  outputState[7],
                     outputState[8], outputState[9], outputState[10], outputState[11]);

   Serial.write(txBuffer);
   logPrintf("Sending message to arduino: %s",txBuffer)
   lastMsgTx = millis();
}

void recvMsg() {

   // Get serial data
   while (Serial.available()) {
      if ( rxCount == 50 ) rxCount = 0;

      c = Serial.read();
      rxBuffer[rxCount++] = c;
      rxBuffer[rxCount] = '\0';
   }

   // Check for incoming message
   if ( rxCount > 7 && rxBuffer[rxCount-1] == '\n' ) {

      // Parse string
      tmp = sscanf(rxBuffer,"%s %i %i %i %i %i %i %i %i %i %i %i %i",
                   mark, &(statusState[0]), &(statusState[1]), &(statusState[2]), &(statusState[3]),
                         &(statusState[4]), &(statusState[5]), &(statusState[6]), &(statusState[7]),
                         &(statusState[8]), &(statusState[9]), &(statusState[10]), &(statusState[11]));

      // Check marker
      if ( tmp == 13 && strcmp(mark,"STATUS") == 0 ) logPrintf("Got arduino message: %s",rxBuffer)
      rxCount = 0;
   }
}

// Initialize
void setup() {

   lastMsgTx   = millis();
   lastDigital = millis();

   // Start and connect to WIFI
   WiFi.mode(WIFI_STA);
   WiFi.setSleepMode(WIFI_NONE_SLEEP);
   WiFi.begin(ssid, password);

   // Connection to arduino
   Serial.begin(9600);

   while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      delay(5000);
      ESP.restart();
   }
   delay(1000);

   // Start logging
   //logUdp.begin(locPort);
   delay(1000);
   logPrintf("System booted!")

   // Firmware download start callback
   ArduinoOTA.onStart([]() {
      if (ArduinoOTA.getCommand() == U_FLASH) {
         logPrintf("Starting sketch firmware upload")
      }
      else { // U_SPIFFS
         logPrintf("Starting filesystem firmware upload")
      }
   });

   // Firmware download done callback
   ArduinoOTA.onEnd([]() {
      logPrintf("Finished firmware upload. Rebooting....")
   });

   // Firmware download progress
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      logPrintf("Download Progress: %u%%",(progress / (total / 100)))
   });

   ArduinoOTA.onError([](ota_error_t error) {
      if      (error == OTA_AUTH_ERROR)    {logPrintf("Download Failed: Auth Failed")}
      else if (error == OTA_BEGIN_ERROR)   {logPrintf("Download Failed: Begin Failed")}
      else if (error == OTA_CONNECT_ERROR) {logPrintf("Download Failed: Connect Failed")}
      else if (error == OTA_RECEIVE_ERROR) {logPrintf("Download Failed: Receive Failed")}
      else if (error == OTA_END_ERROR)     {logPrintf("Download Failed: End Failed")}
   });

   ArduinoOTA.begin();
   server.begin();

   rxCount = 0;

   // Init relays
   for (x=0; x < OutputCount*2; x++) {
      outputState[x] = 0;
      outputTime[x] = millis();
   }

   sendMsg();
}


void wwwClient() {
   WiFiClient client = server.available();
   tmp = 0;

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

                  if (header.indexOf("/command/") >= 0) {

                     for (x=0; x < OutputCount; x++) {
                        onChan  = OutputOn[x];
                        offChan = OutputOff[x];

                        // Test for channel on
                        sprintf(tmpStr,"/command/on/ch%i",x);
                        if (header.indexOf(tmpStr) >= 0) {
                           outputState[onChan] = 100;
                           outputTime[onChan] = millis();
                           logPrintf("Turning on relay %i",onChan)
                        }

                        // Test for channel off
                        sprintf(tmpStr,"/command/off/ch%i",x);
                        if (header.indexOf(tmpStr) >= 0) {
                           outputState[offChan] = 100;
                           outputTime[offChan] = millis();
                           logPrintf("Turning on relay %i",offChan)
                        }
                     }

                     // Test for All on
                     sprintf(tmpStr,"/command/on/all",x);
                     if (header.indexOf(tmpStr) >= 0) {
                        for (x=0; x < OutputCount; x++) {
                           onChan  = OutputOn[x];

                           outputState[onChan] = 100;
                           outputTime[onChan] = millis();
                           logPrintf("Turning on relay %i",onChan)
                        }
                     }

                     // Test for Group all off
                     sprintf(tmpStr,"/command/off/all",x);
                     if (header.indexOf(tmpStr) >= 0) {
                        for (x=0; x < OutputCount; x++) {
                           offChan = OutputOff[x];
                           outputState[offChan] = 100;
                           outputTime[offChan] = millis();
                           logPrintf("Turning on relay %i",offChan)
                        }
                     }

                     client.println("<!DOCTYPE html><html>");
                     client.println("<head><meta http-equiv=\"refresh\" content=\"0; URL=/\" /></head>");
                     client.println("</html>");
                     client.println();
                     tmp = 1;

                  } else {

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
                     client.println("<h1>Gym Light Control</h1>");
                     client.println("<table>");

                     // Display on buttons
                     for(x=0; x < OutputCount; x++) {
                        client.println("<tr>");
                        sprintf(tmpStr,"<th><b>Switch %i</b></th>",x+1);
                        client.println(tmpStr);
                        sprintf(tmpStr,"<th><a href=\"/command/on/ch%i\"><button class=\"button\">ON</button></a></th>",x);
                        client.println(tmpStr);
                        sprintf(tmpStr,"<th><a href=\"/command/off/ch%i\"><button class=\"button\">OFF</button></a></th>",x);
                        client.println(tmpStr);
                        client.println("</tr>");
                     }

                     client.println("<tr>");
                     sprintf(tmpStr,"<th><b>All</b></th>",x);
                     client.println(tmpStr);
                     sprintf(tmpStr,"<th><a href=\"/command/on/all\"><button class=\"button\">ON</button></a></th>",x);
                     client.println(tmpStr);
                     sprintf(tmpStr,"<th><a href=\"/command/off/all\"><button class=\"button\">OFF</button></a></th>",x);
                     client.println(tmpStr);
                     client.println("</tr>");
                     client.println("</table>");

                     client.print("WIFI Strength: ");
                     client.print(wifiPct());
                     client.println(" %");

                     client.println("</center>");
                     client.println("</body></html>");

                     // The HTTP response ends with another blank line
                     client.println();
                  }

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

   if ( tmp == 1 ) sendMsg();
}


// Service Loop
void loop() {
   ArduinoOTA.handle();

   currTime = millis();

   // Attempt to receive input message
   recvMsg();

   // Refresh digital values
   if ( (currTime - lastDigital) > DigitalPeriod) {
      lastDigital = currTime;

      sprintf(valueStr,"%i",wifiPct());
      logPrintf("Wifi strenth = %s",valueStr);
   }

   if (( currTime - lastMsgTx ) > MsgTxPeriod) tmp = 1;
   else tmp = 0;

   for (x=0; x < OutputCount*2; x++) {
      if (outputState[x] == 100 && ((currTime - outputTime[x]) > PulsePeriod) ) {
         outputState[x] = 0;
         outputTime[x] = millis();
         logPrintf("Turning off relay %i",x)
         tmp = 1;
      }
   }

   if ( tmp == 1 ) sendMsg();
   wwwClient();
}

