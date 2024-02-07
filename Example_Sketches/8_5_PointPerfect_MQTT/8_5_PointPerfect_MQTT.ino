/*
  SparkFun RTK EVK Test Sketch

  License: MIT. Please see LICENSE.md for more details

  This example demonstrates how to connect to PointPerfect using MQTT on the LARA-R6001D
  and feed localized corrections to the ZED-F9P.

  You need a valid IP or IP+L-Band subscription for this example.
  Log in to Thingstream and copy your Location Thing Client ID and certificates into secrets.h.

  This example is based partly on Michael Ammann (@mazgch)'s HPG Solution: https://github.com/mazgch/hpg
  Thank you Michael
*/

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "secrets.h"
#include "HW.h"
#include "GNSS.h"
#include "LBand.h"
#include "LARA-R6.h"

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  initPins(); // Initialize the IO pins and start I2C - see HW.ino

  delay(1000); // Wait for the ESP32

  initSerial(Serial, 115200); // Initialize the serial console - see HW.ino

  if (!initGNSS()) // Initialize the GNSS - see GNSS.h
  {
    console->println(F("Could not initialize the GNSS! Freezing..."));
    while (1)
      ;
  }
  console->println(F("GNSS is ready"));

  // Disable PMP messages on the NEO-D9S UART1 to prevent the ZED getting correction data twice!
  if (!initLBand()) //see LBand.h
  {
    console->println(F("Could not initialize the L-Band! Freezing..."));
    while (1)
      ;
  }

  if (!initLARA()) // Initialize the LARA-R6 - see LARA-R6.h
  {
    console->println(F("Could not initialize the LARA! Freezing..."));
    while (1)
      ;
  }
  console->println(F("LARA-R6 is ready"));

  console->println(F(">>> Press any key to connect to toggle the MQTT connection to PointPerfect <<<"));
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void loop()
{
  myGNSS.checkUblox();     // Check for the arrival of new GNSS data and process it.
  myGNSS.checkCallbacks(); // Check if any GNSS callbacks are waiting to be processed.

  mqttTask(keyPressed()); // This task handles the MQTT connection
  myLARA.bufferedPoll();  // Process any URC messages from the LARA
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
