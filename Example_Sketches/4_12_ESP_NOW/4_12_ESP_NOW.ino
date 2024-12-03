/**********************************************************************
  4_12_ESP_NOW.ino

  Example sketch to demonstrate ESP_NOW startup
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <Arduino.h>    // Built-in
#include <ESPmDNS.h>    // Built-in
#include <MacAddress.h> // Built-in
#include <Network.h>    // Built-in
#include <WiFi.h>       // Built-in

#include "esp_now.h"        // IDF built-in
#include "esp_mac.h"        // IDF built-in
#include <esp_wifi.h>       // IDF built-in
#include "esp_wifi_types.h" // IDF built-in

#include <secrets.h>    // Host name, SSIDs and passwords

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

#define systemPrintf    Serial.printf
#define systemPrintln   Serial.println

//****************************************
// WiFi class
//****************************************

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    systemPrintln();
    systemPrintf(__FILE__ "\r\n");
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    espNowTest();
}
