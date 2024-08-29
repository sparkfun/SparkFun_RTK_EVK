//----------------------------------------
// Initialize WiFi
void wifiSetup()
{
    // Initialize WiFi
    wifiPriority = networkPriorityGet(&WiFi.STA);
    networkMarkOffline(wifiPriority);
    wifiMulti.addAP(wifiSSID, wifiPassword);
    wifiMulti.run();
}

//----------------------------------------
void wifiEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    char ssid[sizeof(info.wifi_sta_connected.ssid) + 1];
    IPAddress ipAddress;

    // Take the network offline if necessary
    if (networkIsInterfaceOnline(wifiPriority)
        && (event != ARDUINO_EVENT_WIFI_STA_GOT_IP)
        && (event != ARDUINO_EVENT_WIFI_STA_GOT_IP6))
    {
        networkMarkOffline(wifiPriority);
    }

    // Handle the event
    switch (event)
    {
    default:
        Serial.printf("ERROR: Unknown WiFi event: %d\r\n", event);
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
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        memcpy(ssid, info.wifi_sta_connected.ssid, info.wifi_sta_connected.ssid_len);
        ssid[info.wifi_sta_connected.ssid_len] = 0;
        Serial.printf("WiFi STA connected to %s\r\n", ssid);
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        memcpy(ssid, info.wifi_sta_disconnected.ssid, info.wifi_sta_disconnected.ssid_len);
        ssid[info.wifi_sta_disconnected.ssid_len] = 0;
        Serial.printf("WiFi STA disconnected from %s\r\n", ssid);
        // wifi_event_sta_disconnected_t info.wifi_sta_disconnected;
        break;

    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("WiFi STA Auth Mode Changed");
        // wifi_event_sta_authmode_change_t info.wifi_sta_authmode_change;
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        ipAddress = WiFi.localIP();
        Serial.print("WiFi STA Got IPv4: ");
        Serial.println(ipAddress);
        networkMarkOnline(wifiPriority);
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.print("WiFi STA Got IPv6: ");
        Serial.println(ipAddress);
        networkMarkOnline(wifiPriority);
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("WiFi STA Lost IP");
        break;
    }
}
