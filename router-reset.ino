////////////////////////////////////////////////////////////////////////
// Router reset application
// This program uses a relay to temporarily pull power from a device
// and then restore power 30 seconds later.  The intended use is to reset
// my DSL router when a web-directed reset isn't fixing issues.
// Has many other potential uses.
////////////////////////////////////////////////////////////////////////

#include <SoftwareSerial.h>

//#define DEBUG 1

const int RELAY = 6;

// 30 seconds, in milliseconds
const unsigned long RESET_DELAY = 35000;
const unsigned long UPDATE_DELAY = 1000;

// Bluetooth communication using these two pins.
SoftwareSerial BT (8, 9);  // Rx, Tx pins, from the Arduino perspective  (Tx, Rx on BT)

int byteCount = 0;

// value received from the sending app
int resetCommand = 0;

bool resetCmdRcvd = false;
unsigned long resetTime = 0;
unsigned long powerTime = 0;
unsigned long lastExec = 0;
unsigned long lastUpdate = 0;

const char RESET_RECEIVED[] = "Reset Received - wait 30 seconds\n";
const char POWER_RESTORED[] = "Power restored\n";
const char INVALID_COMMAND[] = "Invalid command received\n";
const char REMAINING[] = " seconds remaining\n";

char statusBuffer[80];

float temp;

void setup ()
{
   pinMode (RELAY, OUTPUT);
   
   BT.begin (9600);
   Serial.begin (9600);

}

void loop ()
{
#if DEBUG
   byteCount = Serial.available ();
#else
   byteCount = BT.available ();
#endif

   lastExec = millis ();

   // listen for incoming data from the BT interface.
   if (byteCount > 0)
   {
#if DEBUG
      resetCommand = Serial.parseInt();
#else
      resetCommand = BT.parseInt();
#endif

      if (resetCommand == 1)
      {
         resetCmdRcvd = true;

         // Using a "normally closed" relay, write HIGH to open the circuit
         digitalWrite (RELAY, HIGH);

         // get the current time, in order to know when to restore power
         powerTime = millis () + RESET_DELAY;

         // notify the sender that the command has been received
         BT.write (RESET_RECEIVED);
         Serial.write (RESET_RECEIVED);
      }
      else
      {
         BT.write (INVALID_COMMAND);
         Serial.write (INVALID_COMMAND);
      }
   }

   // Wait until the time to restore power has arrived
   if (resetCmdRcvd && (lastExec >= powerTime))
   {
      resetCmdRcvd = false;
      digitalWrite (RELAY, LOW);
      Serial.write (POWER_RESTORED);
      BT.write (POWER_RESTORED);
   }
   else if (resetCmdRcvd && (lastExec < powerTime))
   {
      // Periodically update the app with the time remaining.
      if (lastExec >= lastUpdate + UPDATE_DELAY)
      {
         lastUpdate = lastExec;
         sprintf (statusBuffer, "%d", (int) round ((float) (powerTime - lastUpdate) / 1000.0));
         strcat (statusBuffer, REMAINING);
         Serial.write (statusBuffer);
         BT.write (statusBuffer);
      }
   }
}

