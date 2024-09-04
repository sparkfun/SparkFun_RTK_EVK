//----------------------------------------
// Initialize Ethernet
void ethernetSetup()
{
    // Initialize SPI
    SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);

    // Initialize Ethernet
    ethernetPriority = networkPriorityGet(&ETH);
    networkMarkOffline(ethernetPriority);
    ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI);
}

//----------------------------------------
// Process the Ethernet events
void ethernetEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    IPAddress ipAddress;

    // Take the network offline if necessary
    if (networkIsInterfaceOnline(ethernetPriority) && (event != ARDUINO_EVENT_ETH_GOT_IP))
        networkMarkOffline(ethernetPriority);

    // Handle the event
    switch (event)
    {
    default:
        Serial.printf("ERROR: Unknown WiFi event: %d\r\n", event);
        break;

    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-eth0");
        break;

    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;

    case ARDUINO_EVENT_ETH_GOT_IP:
        ipAddress = ETH.localIP();
        Serial.print("ETH Got IP: ");
        Serial.println(ipAddress);

        // The network is now available for use
        networkMarkOnline(ethernetPriority);
        break;

    case ARDUINO_EVENT_ETH_LOST_IP:
        Serial.println("ETH Lost IP");
        break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        break;

    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        break;
    }
}
