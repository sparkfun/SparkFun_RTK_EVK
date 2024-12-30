/**********************************************************************
  4_15_ESP_NOW_Client.ino

  Example sketch to test ESP-NOW mode with example 4_13_Muliple_WiFi_Users
**********************************************************************/

#include "ESP32_NOW_Serial.h"
#include "MacAddress.h"
#include "WiFi.h"

#include "esp_wifi.h"

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

const int32_t channelList[] =
{
    1,
    8,
};
const int32_t channelCount = sizeof(channelList) / sizeof(channelList[0]);
int32_t channelIndex;
int32_t channel;

// Set the MAC address of the device that will receive the data
const MacAddress macPeer({0xE0, 0x5A, 0x1B, 0xD8, 0x8F, 0x14});

const wifi_interface_t interface = WIFI_IF_STA;
const char * testData = "ESP-NOW test data\r\n";

ESP_NOW_Serial_Class * nowSerial;

//*********************************************************************
// Entry point for the application
void setup()
{
    Serial.begin(115200);
    Serial.printf("\r\n");
    Serial.printf("%s\r\n", __FILE__);
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    static uint32_t wifiConnectMsec;

    // Connect to WiFi
    if (!nowSerial)
    {
        // Set the mode
        WiFi.mode(WIFI_MODE_STA);
        esp_wifi_set_protocol(interface,
                              WIFI_PROTOCOL_11B
                              | WIFI_PROTOCOL_11G
                              | WIFI_PROTOCOL_11N);

        // Set the WiFi channel
        channel = channelList[channelIndex++];
        if (channelIndex >= channelCount)
            channelIndex = 0;
        WiFi.setChannel(channel, WIFI_SECOND_CHAN_NONE);

        // Start WiFi if not already running
Serial.printf("Starting WiFi\r\n");
        while (!WiFi.STA.started())
            delay(100);

        // Create the ESPNow serial object
        esp_now_init();
        nowSerial = new ESP_NOW_Serial_Class(macPeer, channel, interface);
        if (nowSerial)
        {
Serial.printf("ESP-NOW started\r\n");
            nowSerial->begin(115200);
            wifiConnectMsec = millis();
        }
    }

    // Send the ESP-NOW test data
    else if (nowSerial->availableForWrite())
    {
        size_t bytesWritten;

Serial.printf("Sending ESP-NOW data\r\n");
        bytesWritten = nowSerial->write((uint8_t *)testData, strlen(testData));
Serial.printf("bytesWritten: %ld\r\n", bytesWritten);
        if (bytesWritten > 0)
            nowSerial->flush();
        else
          Serial.println("Failed to send data");

        // Done with the ESP-NOW connection
        disconnect();
        delay(1000);
    }

    // Timeout the ESP-NOW link
    else
    {
        if ((millis() - wifiConnectMsec) >= 500)
            disconnect();
    }
}

//*********************************************************************
// Break ESP-NOW link
void disconnect()
{
Serial.printf("Breaking ESP-NOW Link\r\n");
    // Break the ESP-NOW link
    if (nowSerial)
    {
        esp_now_deinit();
        nowSerial->end();
        delete nowSerial;
        nowSerial = nullptr;
    }

    // Stop WiFi
    WiFi.mode((wifi_mode_t)0);
    delay(100);
}
