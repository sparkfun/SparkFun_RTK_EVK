/*
 WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 5.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA2 encryption. For insecure
 WEP or WPA, change the Wifi.begin() call and use Wifi.setMinSecurity() accordingly.

 Circuit:
 * WiFi shield attached
 * LED attached to pin 5

 created for arduino 25 Nov 2012
 by Tom Igoe

ported for sparkfun esp32
31.01.2017 by Jan Hendrik Berlin

 Updated to support SparkFun RTK EVK, 21 Nov 2024
*/

#include <esp_wifi.h>       // IDF built-in
#include <WiFi.h>
#include "secrets.h"

#define WIFI_MODE           WIFI_MODE_AP
//#define WIFI_MODE           WIFI_MODE_STA

#define SERVER_PORT         80

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

void setup()
{
    IPAddress ipAddress;
    uint8_t protocols;
    const char * ssid;
    esp_err_t status;

    Serial.begin(115200);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    delay(10);

    // We start by connecting to a WiFi network

    if (WIFI_MODE == WIFI_MODE_AP)
    {
        ssid = apSSID;
        WiFi.softAP(apSSID, apPassword, 1, false, 4, false);
    }
    else
    {
        ssid = staSSID;
        WiFi.begin(staSSID, staPassword);
    }
    Serial.printf("%s SSID: %s\r\n",
                  (WIFI_MODE == WIFI_MODE_AP) ? "Soft AP broadcasting"
                                              : "Station connected to",
                  ssid);

    // Determine the protocols that are enabled
    protocols = 0;
    status = esp_wifi_get_protocol(WIFI_IF_AP, &protocols);
    if (status != ESP_OK)
        Serial.printf("ERROR: esp_wifi_get_protocol failed, status: 0x%x!\r\n", status);

    // Make sure the 802.3 protocols are enabled
    protocols |= WIFI_PROTOCOL_11B
               | WIFI_PROTOCOL_11N
//               | WIFI_PROTOCOL_11AX
               | WIFI_PROTOCOL_11G;
    status = esp_wifi_set_protocol(WIFI_IF_AP, protocols);
    if (status != ESP_OK)
        Serial.printf("ERROR: esp_wifi_set_protocol failed, status: 0x%x!\r\n", status);

    while (!(WiFi.AP.started() || (WiFi.STA.started() && WiFi.STA.hasIP())))
    {
        delay(50);
        Serial.print(".");
    }
    Serial.println();

    // Start the web server
    ipAddress = (WIFI_MODE == WIFI_MODE_AP) ? WiFi.softAPIP() : WiFi.localIP();
    serverBegin(ipAddress, SERVER_PORT);
}

void loop()
{
    serverUpdate();
}
