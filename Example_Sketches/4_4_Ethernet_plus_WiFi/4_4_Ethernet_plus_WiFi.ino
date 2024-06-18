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

#define USE_DHCP 0 // Change to 1 to use DHCP

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

esp_netif_t *ethernet_netif = nullptr;

static bool eth_connected = false;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// WiFi

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiGeneric.h>
#include "secrets.h"

WiFiMulti wifiMulti;

esp_netif_t *wifi_netif = nullptr;

static bool wifi_connected = false;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Network Events

void onNetworkEvent(arduino_event_id_t event, arduino_event_info_t info)
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
        // esp_eth_handle_t info.eth_connected;
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.printf("ETH Got IP: '%s'\r\n", esp_netif_get_desc(info.got_ip.esp_netif));
        Serial.println(ETH);
        // ip_event_got_ip_t info.got_ip;
        ethernet_netif = info.got_ip.esp_netif;
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        Serial.printf("ETH Got **IP6**: '%s'\r\n", esp_netif_get_desc(info.got_ip.esp_netif));
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

    case ARDUINO_EVENT_WIFI_OFF:
        Serial.println("WiFi Off");
        break;
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi Ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("WiFi Scan Done");
        // wifi_event_sta_scan_done_t info.wifi_scan_done;
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi STA Started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi STA Stopped");
        wifi_connected = false;
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("WiFi STA Connected");
        // wifi_event_sta_connected_t info.wifi_sta_connected;
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("WiFi STA Disconnected");
        // wifi_event_sta_disconnected_t info.wifi_sta_disconnected;
        wifi_connected = false;
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("WiFi STA Auth Mode Changed");
        // wifi_event_sta_authmode_change_t info.wifi_sta_authmode_change;
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.printf("WiFi STA Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif));
        wifi_netif = info.got_ip.esp_netif;
        wifi_connected = true;
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.printf("WiFi STA Got **IP6**: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif));
        wifi_connected = true;
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("WiFi STA Lost IP");
        wifi_connected = false;
        break;
    
    default:
        break;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Network Client

NetworkClient *networkClient = nullptr;

bool testClient(NetworkClient *client, const char *host, uint16_t port)
{
    Serial.print("\r\nConnecting to ");
    Serial.println(host);

    if (!client->connect(host, port))
    {
        Serial.println("Connection failed\r\n");
        client->stop();
        return false;
    }

    client->printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);

    while (client->connected() && !client->available())
        ;

    if (client->available())
    {
        Serial.println("Client connected");
    }

    const char findMe[] = "The document has moved";
    int foundChar = 0;
    bool success = false;

    while (client->available())
    {
        char c = client->read();
        //Serial.write(c);

        if (c == findMe[foundChar])
        {
          foundChar++;
          if (foundChar == strlen(findMe))
            success = true;
        }
        else
        {
          foundChar = 0;
        }
    }

    if (success)
      Serial.println("Success");
    else
      Serial.println("FAIL");

    Serial.println("Closing connection\r\n");

    client->stop();

    return (success);
}

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

    Network.onEvent(onNetworkEvent);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    
    // Don't call wifiMulti.run() here. The loop will do it

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Begin SPI

    SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Begin ETHernet

    if (ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI))
    {
#if (!USE_DHCP)
      ETH.config("192.168.0.123", "192.168.0.1", "255.255.255.0", "194.168.4.100");
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

    networkClient = new NetworkClient;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void loop()
{
  static bool previous_wifi_connected = false;
  static bool previous_eth_connected = false;

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Test the network every 20s by connecting to Google

  static unsigned long lastTest = 0;

  if (millis() > (lastTest + 20000))
  {
    lastTest = millis();

    bool success = false;

    if (networkClient)
    {
      success = testClient(networkClient, "google.com", 80);
    }
  }

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Attempt to (re)connect to WiFi if needed every 60s

  static unsigned long lastWiFiReconnect = 0;

  if ((previous_wifi_connected == false) && (wifi_connected == false) && (millis() > (lastWiFiReconnect + 60000)))
  {
    lastWiFiReconnect = millis();

    Serial.println("Attempting to (re)connect to WiFi...");

    wifiMulti.addAP(ssid, password);
    wifiMulti.run();    
  }

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Print which networks are connected every 2s. Change the default as needed

  static unsigned long lastPrint = 0;

  if (millis() > (lastPrint + 2000))
  {
    lastPrint = millis();

    Serial.printf("WiFi is %sconnected. ETH is %sconnected\r\n", wifi_connected ? "" : "not ", eth_connected ? "" : "not ");

    if ((previous_wifi_connected == false) && (wifi_connected == true) && (eth_connected == false) && wifi_netif)
    {
      // WiFi has (re)connected. Ethernet is down. Make WiFi the default.
      Serial.println("WiFi has (re)connected. Making it the default");
      esp_netif_set_default_netif(wifi_netif);
    }
    
    if ((previous_wifi_connected == true) && (wifi_connected == false) && ethernet_netif)
    {
      // WiFi has gone down. Ensure ETH is the default.
      Serial.println("WiFi has disconnected. Making ETH the default");
      esp_netif_set_default_netif(ethernet_netif);

      WiFi.mode(WIFI_OFF);
      wifi_netif = nullptr;
    }
    
    if ((previous_eth_connected == false) && (eth_connected == true) && (wifi_connected == false) && ethernet_netif)
    {
      // Ethernet has (re)connected. WiFi is down. Make Ethernet the default.
      Serial.println("ETH has (re)connected. Making it the default");
      esp_netif_set_default_netif(ethernet_netif);
    }

    if ((previous_eth_connected == true) && (eth_connected == false) && wifi_netif)
    {
      // Ethernet has gone down. Ensure WiFi is the default.
      Serial.println("ETH has disconnected. Making WiFi the default");
      esp_netif_set_default_netif(wifi_netif);
    }

    previous_wifi_connected = wifi_connected;
    previous_eth_connected = eth_connected;
  }
}
