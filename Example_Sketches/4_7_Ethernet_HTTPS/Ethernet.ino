bool ethernetOnline = false;

//----------------------------------------
// Determine if Ethernet controller has an IP address
bool ethernetIsInternetAvailable()
{
    return ethernetOnline;
}

//----------------------------------------
// Process the Ethernet events
void ethernetEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    IPAddress ipAddress;

    switch (event)
    {
    default:
        Serial.printf("ERROR: Unknown WiFi event: %d\r\n", event);
        break;

    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-eth0");
        ethernetOnline = false;
        break;

    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;

    case ARDUINO_EVENT_ETH_GOT_IP:
        ipAddress = ETH.localIP();
        Serial.print("ETH Got IP: ");
        Serial.println(ipAddress);
        ethernetOnline = true;
        break;

    case ARDUINO_EVENT_ETH_LOST_IP:
        Serial.println("ETH Lost IP");
        ethernetOnline = false;
        break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        ethernetOnline = false;
        break;

    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        ethernetOnline = false;
        break;
    }
}
