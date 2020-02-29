// include the library code:
#include <string.h>
#include <avr/wdt.h>

// Setup pins
const unsigned int RelayCount           = 12;
const unsigned int RelayPin[RelayCount] = {2,3,4,5,6,7,8,9,10,11,12,13};

// Setup constants
const unsigned int  RelayOff       = 0;
const unsigned int  RelayOn        = 100;

///////////////////////////////////////////////////////////////////////////

unsigned int rxCount;

char txBuffer[50];
char rxBuffer[50];
char c;

// Variables
unsigned int  curRelState[RelayCount];
unsigned int  reqRelState[RelayCount];

unsigned int  x;
unsigned int  txReq;
char          mark[20];
int           ret;

// Initialize
void setup() {

   // Setup and init outputs
   for (x=0; x < RelayCount; x++) {
      digitalWrite(RelayPin[x],HIGH);
      pinMode(RelayPin[x],OUTPUT);
      curRelState[x] = 0;
   }

   // Init variables
   rxCount      = 0;

   // start serial
   Serial.begin(9600);

   // Enable watchdog timer
   wdt_enable(WDTO_8S);
}

// Main loop
void loop() {
   txReq = 0;

   // Get serial data
   while (Serial.available()) {
      if ( rxCount == 50 ) rxCount = 0;

      c = Serial.read();
      rxBuffer[rxCount++] = c;
      rxBuffer[rxCount] = '\0';
   }

   // Check for incoming message
   if ( rxCount > 6 && rxBuffer[rxCount-1] == '\n') {

      // Parse string
      ret = sscanf(rxBuffer,"%s %i %i %i %i %i %i %i %i %i %i %i %i", 
                   mark, &(reqRelState[0]), &(reqRelState[1]), &(reqRelState[2]),
                   &(reqRelState[3]), &(reqRelState[4]),  &(reqRelState[5]),
                   &(reqRelState[6]), &(reqRelState[7]),  &(reqRelState[8]),
                   &(reqRelState[9]), &(reqRelState[10]), &(reqRelState[11]));

      // Check marker
      if ( ret == 13 && strcmp(mark,"STATE") == 0 ) {
         for (x=0; x < RelayCount; x++) curRelState[x] = reqRelState[x];
         txReq = 1;
      }
      rxCount = 0;
   }

   // Update relay output states
   for (x=0; x < RelayCount; x++) {
      if ( curRelState[x] == RelayOn ) digitalWrite(RelayPin[x],LOW);
      else digitalWrite(RelayPin[x],HIGH);
   }

   // Transmit if requested
   if ( txReq == 1 ) {

      sprintf(txBuffer,"STATUS %i %i %i %i %i %i %i %i %i %i %i %i\n",
                       curRelState[0], curRelState[1], curRelState[2],  curRelState[3],
                       curRelState[4], curRelState[5], curRelState[6],  curRelState[7],
                       curRelState[8], curRelState[9], curRelState[10], curRelState[11]);

      Serial.write(txBuffer);
   }

   // Pet the dog
   wdt_reset();
}

