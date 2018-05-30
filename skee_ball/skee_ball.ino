
const int in_pin[] = {4, 5, 6, 7, 8};
const int rst_pin  = 11;
const int out_pin  = 13;
const int out_time = 1000;

int in_hit[5];
int out_cnt;
int one_hit;

// the setup routine runs once when you press reset:
void setup() {             
   int i;

   Serial.begin(9600);
   Serial.println("Skee Ball Started");

   for (i=0; i < 5; i++) {
      pinMode ( in_pin[i], INPUT );
      digitalWrite(in_pin[i], HIGH);
      in_hit[i] = 0;
   }

   pinMode ( rst_pin, INPUT );
   digitalWrite(rst_pin, HIGH);

   pinMode ( out_pin, OUTPUT );
   digitalWrite(out_pin, HIGH);
   out_cnt = 0;
   one_hit = 0;
}

void loop() {
   int i;

   for (i=0; i < 5; i++) {
      if ( (! digitalRead(in_pin[i])) &&  ( ! in_hit[i] ) ) {
         out_cnt = out_time;
         in_hit[i] = 1;
         one_hit = 1;
      }
   }

   if ( (! digitalRead(rst_pin)) && one_hit ) {

      Serial.println("Got roll: ");

      for (i=0; i < 5; i++) {
         Serial.print((i+1)*10);
         Serial.print(" switch = ");
         Serial.print(in_hit[i]);
         Serial.println("");
         in_hit[i] = 0;
      }
      out_cnt = 0;
      one_hit = 0;
      digitalWrite(out_pin, HIGH);
   }

   else if ( out_cnt > 0 ) {
      digitalWrite(out_pin, LOW);
      out_cnt--;
   }

   else digitalWrite(out_pin, HIGH);

}

