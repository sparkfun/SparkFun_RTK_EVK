// bug-NetworkEvents-removeEvent.ino

#define wifiSSID        "Your_WiFi_SSID"
#define wifiPassword    "Your_WiFi_Password"

#include <WiFi.h>

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

//----------------------------------------
// System initialization
void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("bug-NetworkEvents-removeEvent.ino");

    // Listen for wifi events
    Serial.printf("Registering callback handler: %p\r\n", (void *)wifiEvent);
    Network.onEvent(wifiEvent);
    Network.onEvent(wifiEvent);
    Network.onEvent(wifiEvent);
    Network.onEvent(wifiEvent);

    // Start displaying the events
    Serial.println("==================== Receiving 4 events / actual event ====================");

    // Start WiFi
    WiFi.begin(wifiSSID, wifiPassword);
}

//----------------------------------------
// Main loop
void loop()
{
    static bool eventRemoved;
    static uint32_t lastTimeMillis;
    static bool wifiState;

    uint32_t currentMillis = millis();
    if ((currentMillis - lastTimeMillis) >= (10 * 1000))
    {
        lastTimeMillis = currentMillis;

        // Toggle the Wifi state
        wifiState ^= 1;
        if (wifiState)
        {
            Serial.println("---------- WiFi.disconnect ----------");
            WiFi.disconnect(true);
        }
        else
        {
            Serial.println("---------- WiFi.begin ----------");
            WiFi.begin(wifiSSID, wifiPassword);
        }
    }
    if ((!eventRemoved) && (currentMillis >= (60 * 1000)))
    {
        // Removing the WiFi event handler
        // This should remove them all!!!!!!
        Network.removeEvent(wifiEvent);
        eventRemoved = true;
        Serial.println("==================== Removed all the WiFi Event Handlers ====================");
        Serial.println("==================== No Event Output Should Be Displayed ====================");
    }
}

//----------------------------------------
void wifiEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    char ssid[sizeof(info.wifi_sta_connected.ssid) + 1];
    IPAddress ipAddress;

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
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.print("WiFi STA Got IPv6: ");
        Serial.println(ipAddress);
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("WiFi STA Lost IP");
        break;
    }
}
