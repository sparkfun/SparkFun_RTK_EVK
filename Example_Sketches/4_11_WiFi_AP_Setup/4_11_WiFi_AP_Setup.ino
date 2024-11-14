/**********************************************************************
  4_11_WiFi_AP_Setup.ino

  Example sketch to demonstrate WiFi AP setup
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <Arduino.h>    // Built-in
#include <ESPmDNS.h>    // Built-in
#include <esp_wifi.h>   // IDF built-in
#include <Network.h>    // Built-in
#include <WiFi.h>       // Built-in

#include <secrets.h>    // Host name, SSIDs and passwords

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

#define systemPrintf    Serial.printf
#define systemPrintln   Serial.println

#define SERVER_PORT     80

//****************************************
// WiFi support
//****************************************

typedef uint8_t WIFI_CHANNEL_t;
typedef uint16_t WIFI_START_LIST_t;

bool debug;
bool display;
bool mDnsRunning;
bool softApHasIp;
bool softApRunning;
int softApStationCount;

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
    wifiUpdate();
    serverUpdate();
}
