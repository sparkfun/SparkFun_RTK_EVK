/*
  SparkFun RTK EVK Test Sketch

  W5500 Ethernet using the Espressif ESP32 ETH. Based on:
  https://github.com/espressif/arduino-esp32/blob/master/libraries/Ethernet/examples/ETH_W5500_Arduino_SPI/ETH_W5500_Arduino_SPI.ino

  Select ESP32 Wrover as the board

  NOTE: this requires arduino-esp32 v3.0.0

  Tested with v3.0.1:
  https://github.com/espressif/arduino-esp32/releases/tag/3.0.1

  See: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
  Add this link to the Additional Boards Manager URLs: https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json

  See also:
  https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html
  https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#functional-changes

  License: MIT. Please see LICENSE.md for more details

  ESP32-WROVER-IE Pin Allocations:
  D0  : Boot + Boot Button
  D1  : Serial TX (CH340 RX)
  D2  : STAT LED
  D3  : Serial RX (CH340 TX)
  D4  : SD CS
  D5  : GNSS Time Pulse - via 74HC4066 switch and PWREN
  D12 : SDA2 - Qwiic OLED - via 74HC4066 switch and PWREN
  D13 : Serial1 TX - LARA_TXDI
  D14 : Serial1 RX - LARA RXDO
  D15 : SCL2 - Qwiic OLED - via 74HC4066 switch and PWREN
  D16 : N/A
  D17 : N/A
  D18 : SPI SCK
  D19 : SPI POCI
  D21 : I2C SDA
  D22 : I2C SCL
  D23 : SPI PICO
  D25 : Serial2 RX - ZED-F9P TXO
  D26 : LARA Power On
  D27 : Ethernet Chip Select
  D32 : PWREN
  D33 : Serial2 TX - ZED-F9P RXI
  A34 : LARA Network Indicator
  A35 : Board Detect (3.0V)
  A36 : SD Card Detect
  A39 : Ethernet Interrupt
*/

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// arduino-esp32 Ethernet

#include <SPI.h>
#include <ETH.h>

const int STAT_LED = 2;
const int SD_CS = 4;         // Chip select for the microSD card
const int GNSS_INT = 5;      // ZED_F9P time pulse interrupt
const int SDA_2 = 12;        // OLED
const int SERIAL1_TX = 13;   // LARA_TXDI
const int SERIAL1_RX = 14;   // LARA RXDO
const int SCL_2 = 15;        // OLED
const int SDA_1 = 21;        // ZED-F9P and NEO-D9S
const int SCL_1 = 22;        // ZED-F9P and NEO-D9S
const int SERIAL2_RX = 25;   // ZED-F9P TXO
const int LARA_PWR = 26;     // LARA_PWR_ON - inverted - set LARA_PWR high to pull LARA_PWR_ON low
const int ETHERNET_CS = 27;  // Chip select for the WizNet W5500
const int PWREN = 32;        // 74HC4066 switch Enable - pull high to enable SCL2/SDA2 and GNSS_INT
const int SERIAL2_TX = 33;   // ZED-F9P RXI
const int LARA_NI = 34;      // LARA Network Indicator - only valid when the LARA is powered on
const int SD_PRESENT = 36;   // microSD card card present - from the microSD socket switch
const int ETHERNET_INT = 39; // WizNet W5500 interrupt

#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 0
#define ETH_PHY_CS ETHERNET_CS
#define ETH_PHY_IRQ ETHERNET_INT
#define ETH_PHY_RST -1

#define ETH_SPI_SCK 18
#define ETH_SPI_MISO 19
#define ETH_SPI_MOSI 23

static bool eth_connected = false;

void onEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-eth0");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.printf("ETH Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif));
        Serial.println(ETH);
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_LOST_IP:
        Serial.println("ETH Lost IP");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}

void testETHClient(const char *host, uint16_t port)
{
    Serial.print("\nConnecting to ");
    Serial.println(host);

    NetworkClient client;
    if (!client.connect(host, port))
    {
        Serial.println("Connection failed");
        return;
    }

    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);

    while (client.connected() && !client.available())
        ;

    if (client.available())
    {
        Serial.println("Client connected");
    }

    while (client.available())
    {
        Serial.write(client.read());
    }

    Serial.println("\r\nClosing connection\r\n");

    client.stop();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// WiFi

#include <WiFi.h>
#include <WiFiMulti.h>
#include "secrets.h"

WiFiMulti wifiMulti;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    pinMode(ETHERNET_CS, OUTPUT);
    digitalWrite(ETHERNET_CS, HIGH);
    pinMode(GNSS_INT, INPUT);
    pinMode(STAT_LED, OUTPUT);
    digitalWrite(STAT_LED, HIGH);
    pinMode(PWREN, OUTPUT);
    digitalWrite(PWREN, HIGH);

    delay(1000);

    Serial.begin(115200);
    Serial.println("SparkFun RTK EVK - Test Sketch");

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Begin WiFi

    Serial.print("Connecting to local WiFi");
  
    wifiMulti.addAP(ssid, password);
    if (wifiMulti.run() == WL_CONNECTED)
    {
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("WiFi NOT connected!");
    }
    Serial.println();
  

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Begin SPI

    SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Begin ETHernet

    //Network.onEvent(onEvent);

    ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI);

}

void loop()
{
    static unsigned long lastETHTest = 0;

    //if (eth_connected && (millis() > (lastETHTest + 30000)))
    if (ETH.linkUp() && ETH.hasIP()  && (millis() > (lastETHTest + 30000)))
    {
        Serial.println("Testing ETH");
        testETHClient("google.com", 80);
        lastETHTest = millis();
    }
}
