/*
    ESP-NOW Serial Example - Unicast transmission
    Lucas Saavedra Vaz - 2024
    Send data between two ESP32s using the ESP-NOW protocol in one-to-one (unicast) configuration.
    Note that different MAC addresses are used for different interfaces.
    The devices can be in different modes (AP or Station) and still communicate using ESP-NOW.
    The only requirement is that the devices are on the same Wi-Fi channel.
    Set the peer MAC address according to the device that will receive the data.

    Example setup:
    - Device 1: AP mode with MAC address F6:12:FA:42:B6:E8
                Peer MAC address set to the Station MAC address of Device 2 (F4:12:FA:40:64:4C)
    - Device 2: Station mode with MAC address F4:12:FA:40:64:4C
                Peer MAC address set to the AP MAC address of Device 1 (F6:12:FA:42:B6:E8)

    The device running this sketch will also receive and print data from any device that has its MAC address set as the peer MAC address.
    To properly visualize the data being sent, set the line ending in the Serial Monitor to "Both NL & CR".
*/

#include "ESP32_NOW_Serial.h"
#include "MacAddress.h"
#include "WiFi.h"

#include "esp_wifi.h"

#define DEFAULT_MODE        WIFI_MODE_STA
//#define DEFAULT_MODE        WIFI_MODE_AP
#define DEFAULT_CHANNEL     1

// Set the MAC address of the device that will receive the data
const MacAddress macDev1({0x48, 0xE7, 0x29, 0x9A, 0xD6, 0x88});
const MacAddress macDev2({0xE0, 0x5A, 0x1B, 0xD8, 0x8F, 0x14});

const char * wifiApSsid = "SoftAP";
const char * wifiApPassword = "Password";

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

ESP_NOW_Serial_Class * nowSerial;

void setup()
{
    int32_t channel;
    wifi_interface_t interface;
    MacAddress macLocal;
    MacAddress macPeer;
    wifi_mode_t mode;

    Serial.begin(115200);
    Serial.printf("\r\n");
    Serial.printf("%s\r\n", __FILE__);

    // Determine the WiFi mode
    mode = WiFi.getMode();
    if (mode == 0)
    {
        mode = DEFAULT_MODE;
        if (mode == WIFI_MODE_AP)
            WiFi.softAP(wifiApSsid, wifiApPassword);
        else
            WiFi.mode(mode);
    }
    Serial.printf("WiFi Mode: %s\r\n", (mode == WIFI_MODE_AP) ? "AP" : "Station");

    // Determine the interface
    interface = (mode == WIFI_MODE_AP) ? WIFI_IF_AP : WIFI_IF_STA;

    // Determine the WiFi channel
    channel = WiFi.channel();
    if (channel == 0)
    {
        channel = DEFAULT_CHANNEL;
        WiFi.setChannel(channel, WIFI_SECOND_CHAN_NONE);
    }
    Serial.printf("Channel: %d\r\n", channel);

    // Start WiFi if not already running
    while (!(WiFi.STA.started() || WiFi.AP.started()))
    {
        delay(100);
    }

    // Get the local MAC address
    macLocal = (mode == WIFI_MODE_AP) ? WiFi.softAPmacAddress() : WiFi.macAddress();
    Serial.printf("MAC Address: %s\r\n", macLocal.toString().c_str());

    // Determine the peer MAC address
    macPeer = (macLocal == macDev1) ? macDev2 : macDev1;

    // Create the ESPNow serial object
    nowSerial = new ESP_NOW_Serial_Class(macPeer, channel, interface);

    // Start the ESP-NOW communication
    Serial.println("ESP-NOW communication starting...");
    nowSerial->begin(115200);
    Serial.println("You can now send data to the peer device using the Serial Monitor.\n");
}

void loop()
{
    while (nowSerial->available())
    {
        Serial.write(nowSerial->read());
    }

    while (Serial.available() && nowSerial->availableForWrite())
    {
        if (nowSerial->write(Serial.read()) <= 0)
        {
          Serial.println("Failed to send data");
          break;
        }
    }

    delay(1);
}
