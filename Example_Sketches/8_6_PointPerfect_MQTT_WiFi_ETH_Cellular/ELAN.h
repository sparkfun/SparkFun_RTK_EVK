#include <SPI.h>
#include <ETH.h>
#include <NetworkClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoMqttClient.h> // https://github.com/arduino-libraries/ArduinoMqttClient

#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 0
#define ETH_PHY_RST -1

static bool eth_connected = false;

const unsigned long ETH_CONNECT_TIMEOUT_MS = 10000;

//NetworkClientSecure lanClient; <- this is in WLAN.h
//HTTPClient httpClient; <- this is in WLAN.h
//MqttClient mqttClient(lanClient); <- this is in WLAN.h

void onEvent_ETH(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        console->println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-eth0");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        console->println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        console->printf("ETH Got IP: '%s'\r\n", esp_netif_get_desc(info.got_ip.esp_netif));
        console->println(ETH);
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_LOST_IP:
        console->println("ETH Lost IP");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        console->println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        console->println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}

// Initialize Ethernet
bool initELAN()
{
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  Network.onEvent(onEvent_ETH);

  ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETHERNET_CS, ETHERNET_INT, ETH_PHY_RST, SPI);

  console->println("ETH begun. Waiting for IP...");

  unsigned long start = millis();
  while ((!eth_connected) && (millis() < (start + ETH_CONNECT_TIMEOUT_MS))) // Wait for up to 10 seconds
  {
    delay(1); // Yield
  }

  return eth_connected;
}

