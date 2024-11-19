/**********************************************************************
  4_8_WiFi_Setup.ino

  Example sketch to demonstrate WiFi setup
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <Arduino.h>    // Built-in
#include <ESPmDNS.h>    // Built-in
#include <Network.h>    // Built-in
#include <WiFi.h>       // Built-in

#include <secrets.h>    // Host name, SSIDs and passwords

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

#define systemPrintf    Serial.printf
#define systemPrintln   Serial.println

//****************************************
// WiFi support
//****************************************

typedef uint8_t WIFI_CHANNEL_t;
typedef uint16_t WIFI_START_LIST_t;

bool apFound;
const char * apPassword;
const char * apSSID;
int authType;
bool debug;
bool display;
bool stationConnected;
bool stationHasIp;
WIFI_CHANNEL_t wifiChannel;
bool wifiScanRunning;
uint32_t wifiTimer;

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    systemPrintln();
    systemPrintf(__FILE__ "\r\n");

    // Initialize WiFi
    wifiBegin();
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    update();
}
