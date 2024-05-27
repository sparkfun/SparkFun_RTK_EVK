/*
  SparkFun RTK EVK Test Sketch

  License: MIT. Please see LICENSE.md for more details

  This example demonstrates how to connect to:
  * Use ZTP to retrieve the certificate and private key needed for MQTT
  * Subscribe to  PointPerfect using MQTT and feed localized corrections to the ZED-F9P
  * Using WiFi or Ethernet or Cellular (LARA-R6)

  You need a valid IP or IP+L-Band subscription for this example.
  Copy your PointPerfect token into secrets.h.

  Note: this example is written for arduino-esp32 v3.0.0 (RC3).
  When compiling with the CLI, include the extra compiler flags -MMD and -c. E.g.:
  --build-property "compiler.cpp.extra_flags=-MMD -c \"-DENABLE_DEVELOPER=true\""

  This example is based partly on Michael Ammann (@mazgch)'s HPG Solution: https://github.com/mazgch/hpg
  Thank you Michael
*/

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

typedef enum
{
  EVK_IF_WIFI = 0,
  EVK_IF_ETH,
  EVK_IF_LARA
} EVKInterfaceTypes;
EVKInterfaceTypes EVKInterfaceType = EVK_IF_LARA; // Change this as needed

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <ArduinoJson.h> //http://librarymanager/All#Arduino_JSON_messagepack v6.19.4

#include "secrets.h"
#include "HW.h"
#include "CONFIG.h"
#include "GNSS.h"
#include "LBand.h"
// #include "EVK_WiFi.h"
// #include "EVK_Ethernet.h"
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
  if (!initLBand()) // See LBand.h
  {
    console->println(F("Could not initialize the L-Band! Freezing..."));
    while (1)
      ;
  }

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  if (EVKInterfaceType == EVK_IF_LARA)
  {
    if (!initLARA()) // Initialize the LARA-R6 - see LARA-R6.h
    {
      console->println(F("Could not initialize the LARA! Freezing..."));
      while (1)
        ;
    }
    console->println(F("LARA-R6 is ready. Waiting for ZTP to complete"));

    mqttProvision_LARA();

    unsigned long ztpStart = millis();

    // Wait for the ZTP to complete. clientID will be updated via the HTTP callback
    while ((clientID.length() == 0) && (millis() < (ztpStart + 10000)))
    {
      myLARA.bufferedPoll(); // Process any URC messages from the LARA
      delay(100);
      console->print(".");
    }
    console->println();

    if (clientID.length() == 0)
    {
      console->println(F("ZTP timed out! Freezing..."));
      while (1)
        ;
    }
    else
    {
      console->printf("clientID:   %s\r\n", clientID.c_str());
      console->printf("brokerHost: %s\r\n", brokerHost.c_str());
      console->printf("currentKey: %s\r\n", currentKey.c_str());
      console->printf("nextKey:    %s\r\n", nextKey.c_str());
    }
  }

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  console->println(F("\r\n>>> Press any key to connect to toggle the MQTT connection to PointPerfect <<<\r\n"));
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void loop()
{
  myGNSS.checkUblox();     // Check for the arrival of new GNSS data and process it.
  myGNSS.checkCallbacks(); // Check if any GNSS callbacks are waiting to be processed.

  mqttTask(keyPressed()); // This task handles the MQTT connection

  if (EVKInterfaceType == EVK_IF_LARA)
    myLARA.bufferedPoll(); // Process any URC messages from the LARA
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
