bool wifiConnected;

//----------------------------------------
// Determine if WiFi controller has an IP address
bool wifiIsInternetAvailable()
{
    return wifiConnected;
}

//----------------------------------------
void wifiEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    char ssid[sizeof(info.wifi_sta_connected.ssid) + 1];
    IPAddress ipAddress;

    switch (event)
    {
    default:
        Serial.printf("ERROR: Unknown WiFi event: %d\r\n", event);
        break;

    case ARDUINO_EVENT_WIFI_OFF:
        Serial.println("WiFi Off");
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi Ready");
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("WiFi Scan Done");
        // wifi_event_sta_scan_done_t info.wifi_scan_done;
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi STA Started");
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi STA Stopped");
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        memcpy(ssid, info.wifi_sta_connected.ssid, info.wifi_sta_connected.ssid_len);
        ssid[info.wifi_sta_connected.ssid_len] = 0;
        Serial.printf("WiFi STA connected to %s\r\n", ssid);
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        memcpy(ssid, info.wifi_sta_disconnected.ssid, info.wifi_sta_disconnected.ssid_len);
        ssid[info.wifi_sta_disconnected.ssid_len] = 0;
        Serial.printf("WiFi STA disconnected from %s\r\n", ssid);
        // wifi_event_sta_disconnected_t info.wifi_sta_disconnected;
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("WiFi STA Auth Mode Changed");
        // wifi_event_sta_authmode_change_t info.wifi_sta_authmode_change;
        wifiConnected = false;
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        ipAddress = WiFi.localIP();
        Serial.print("WiFi STA Got IP: ");
        Serial.println(ipAddress);
        wifiConnected = true;
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.println("WiFi STA Got **IP6**");
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("WiFi STA Lost IP");
        wifiConnected = false;
        break;
    }
}
