/**********************************************************************
  Wifi.now

  WiFi layer, supports use by ESP-NOW, soft AP and WiFi station
**********************************************************************/

//****************************************
// Constants
//****************************************

#define WIFI_RECONNECTION_DELAY         1000
#define WIFI_DEFAULT_CHANNEL            1
#define WIFI_IP_ADDRESS_TIMEOUT_MSEC    (15 * 1000)

static const char * wifiAuthorizationName[] =
{
    "Open",
    "WEP",
    "WPA_PSK",
    "WPA2_PSK",
    "WPA_WPA2_PSK",
    "WPA2_Enterprise",
    "WPA3_PSK",
    "WPA2_WPA3_PSK",
    "WAPI_PSK",
    "OWE",
    "WPA3_ENT_192",
};
static const int wifiAuthorizationNameEntries =
    sizeof(wifiAuthorizationName) / sizeof(wifiAuthorizationName[0]);

const char * arduinoEventName[] =
{
    "ARDUINO_EVENT_NONE",
    "ARDUINO_EVENT_ETH_START",
    "ARDUINO_EVENT_ETH_STOP",
    "ARDUINO_EVENT_ETH_CONNECTED",
    "ARDUINO_EVENT_ETH_DISCONNECTED",
    "ARDUINO_EVENT_ETH_GOT_IP",
    "ARDUINO_EVENT_ETH_LOST_IP",
    "ARDUINO_EVENT_ETH_GOT_IP6",
    "ARDUINO_EVENT_WIFI_OFF",
    "ARDUINO_EVENT_WIFI_READY",
    "ARDUINO_EVENT_WIFI_SCAN_DONE",
    "ARDUINO_EVENT_WIFI_STA_START",
    "ARDUINO_EVENT_WIFI_STA_STOP",
    "ARDUINO_EVENT_WIFI_STA_CONNECTED",
    "ARDUINO_EVENT_WIFI_STA_DISCONNECTED",
    "ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE",
    "ARDUINO_EVENT_WIFI_STA_GOT_IP",
    "ARDUINO_EVENT_WIFI_STA_GOT_IP6",
    "ARDUINO_EVENT_WIFI_STA_LOST_IP",
    "ARDUINO_EVENT_WIFI_AP_START",
    "ARDUINO_EVENT_WIFI_AP_STOP",
    "ARDUINO_EVENT_WIFI_AP_STACONNECTED",
    "ARDUINO_EVENT_WIFI_AP_STADISCONNECTED",
    "ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED",
    "ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED",
    "ARDUINO_EVENT_WIFI_AP_GOT_IP6",
    "ARDUINO_EVENT_WIFI_FTM_REPORT",
    "ARDUINO_EVENT_WPS_ER_SUCCESS",
    "ARDUINO_EVENT_WPS_ER_FAILED",
    "ARDUINO_EVENT_WPS_ER_TIMEOUT",
    "ARDUINO_EVENT_WPS_ER_PIN",
    "ARDUINO_EVENT_WPS_ER_PBC_OVERLAP",
    "ARDUINO_EVENT_SC_SCAN_DONE",
    "ARDUINO_EVENT_SC_FOUND_CHANNEL",
    "ARDUINO_EVENT_SC_GOT_SSID_PSWD",
    "ARDUINO_EVENT_SC_SEND_ACK_DONE",
    "ARDUINO_EVENT_PROV_INIT",
    "ARDUINO_EVENT_PROV_DEINIT",
    "ARDUINO_EVENT_PROV_START",
    "ARDUINO_EVENT_PROV_END",
    "ARDUINO_EVENT_PROV_CRED_RECV",
    "ARDUINO_EVENT_PROV_CRED_FAIL",
    "ARDUINO_EVENT_PROV_CRED_SUCCESS",
    "ARDUINO_EVENT_PPP_START",
    "ARDUINO_EVENT_PPP_STOP",
    "ARDUINO_EVENT_PPP_CONNECTED",
    "ARDUINO_EVENT_PPP_DISCONNECTED",
    "ARDUINO_EVENT_PPP_GOT_IP",
    "ARDUINO_EVENT_PPP_LOST_IP",
    "ARDUINO_EVENT_PPP_GOT_IP6",
};
const int arduinoEventNameEntries = sizeof(arduinoEventName) / sizeof(arduinoEventName[0]);

//----------------------------------------------------------------------
// ESP-NOW bringup from example 4_9_ESP_NOW
//   1. Set station mode
//   2. Create nowSerial as new ESP_NOW_Serial_Class
//   3. nowSerial.begin
// ESP-NOW bringup from RTK
//   1. Get WiFi mode
//   2. Set WiFi station mode if necessary
//   3. Get WiFi station protocols
//   4. Set WIFI_PROTOCOL_LR protocol
//   5. Call esp_now_init
//   6. Call esp_wifi_set_promiscuous(true)
//   7. Set promiscuous receive callback [esp_wifi_set_promiscuous_rx_cb(promiscuous_rx_cb)]
//      to get RSSI of action frames
//   8. Assign a channel if necessary, call espnowSetChannel
//   9. Set receive callback [esp_now_register_recv_cb(espnowOnDataReceived)]
//  10. Add peers from settings
//      A. If no peers exist
//          i.   Determine if broadcast peer exists, call esp_now_is_peer_exist
//          ii.  Add broadcast peer if necessary, call espnowAddPeer
//          iii. Set ESP-NOW state, call espnowSetState(ESPNOW_BROADCASTING)
//      B. If peers exist,
//          i.  Set ESP-NOW state, call espnowSetState(ESPNOW_PAIRED)
//          ii. Loop through peers listed in settings, for each
//              a. Determine if peer exists, call esp_now_is_peer_exist
//              b. Add peer if necessary, call espnowAddPeer
//
// In espnowOnDataReceived
//  11. Save ESP-NOW RSSI
//  12. Set lastEspnowRssiUpdate = millis()
//  13. If in ESPNOW_PAIRING state
//      A. Validate message CRC
//      B. If valid CRC
//          i.  Save peer MAC address
//          ii. espnowSetState(ESPNOW_MAC_RECEIVED)
//  14. Else if ESPNOW_MAC_RECEIVED state
//      A. If ESP-NOW is corrections source, correctionLastSeen(CORR_ESPNOW)
//          i.  gnss->pushRawData
//  15. Set espnowIncomingRTCM
//
// ESP-NOW shutdown from RTK
//   1. esp_wifi_set_promiscuous(false)
//   2. esp_wifi_set_promiscuous_rx_cb(nullptr)
//   3. esp_now_unregister_recv_cb()
//   4. Remove all peers by calling espnowRemovePeer
//   5. Get WiFi mode
//   6. Set WiFi station mode if necessary
//   7. esp_wifi_get_protocol
//   8. Turn off long range protocol if necessary, call esp_wifi_set_protocol
//   9. Turn off ESP-NOW. call esp_now_deinit
//  10. Set ESP-NOW state, call espnowSetState(ESPNOW_OFF)
//  11. Restart WiFi if necessary
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Soft AP startup from example 4_11
//  1. Get mode
//  2. Start WiFi event handler
//  3. Set SSID and password, call softApCreate
//  4. Set IP addresses, call softApConfiguration
//  5. esp_wifi_get_protocol
//  6. Set AP protocols, call esp_wifi_set_protocol(b, g, n)
//  7. Set AP mode
//  8. Set host name
//  9. Set mDNS
//
// Soft AP shutdown from example 4_11
//  1. Stop mDNS
//  2. Get mode
//  3. Disable AP mode
//  4. Stop WiFi event handler
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// WiFi station startup from example 4_8
//  1. Get mode
//  2. Start WiFi event handler
//  3. Set WiFi station mode
//  4. Start Wifi scan
//
// In WiFi event handler for scan complete
//  5. Select AP and channel
//  6. Get mode
//  7. Start WiFi station mode
//  8. Set host name
//  9. Disable auto reconnect
// 10. Connect to AP
//
// In WiFi event handler for station connected
// 11. Set stationConnected
//
// In WiFi event handler for GOT_IP or GOT_IP6
// 12. Set stationHasIp
// 13. Save IP address
// 14. Save IP address type
// 15. Display IP address
// 16. Start mDNS
//
// In WiFi event handler for LOST_IP
//  1. Clear stationHasIp
//  2. Display IP address
//
// In WiFi event handler for all other WiFi station events except
// GOT_IP, GOT_IP6 or LOST_IP
//  1. Clear WiFi channel
//  2. Clear stationConnected
//  3. Clear stationHasIp
//  4. For STOP
//      A. Clear timer
//     Else for DISCONNECTED
//      A. Start timer (set to non-zero value)
//
// In wifiUpdate
//  5. When timer fires
//      A. Start scan
//
// WiFi station shutdown from example 4-8
//  1. Get mode
//  2. Exit if WiFi station is not running
//  3. Stop mDNS
//  4. Disconnect from remote AP
//  5. Stop WiFi station mode
//  6. Stop the event handler
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Combining Soft AP shutdown with WiFi shutdown
//  1. Stop mDNS
//  2. Get mode
//  3. Disconnect from remote AP
//  4. Stop necessary modes (AP and station)
//  5. Stop the event handler
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Combining soft AP starting with WiFi station starting, do the following
// synchronously:
//  1. Get mode
//  2. Start WiFi event handler
//  3. Set AP SSID and password, call softApCreate
//  4. Set AP IP addresses, call softApConfiguration
//  5. Get the AP protocols, call esp_wifi_get_protocol
//  6. Set AP protocols, call esp_wifi_set_protocol(b, g, n)
//  7. Set the necessary WiFi modes
//  8. Set AP host name
//  9. If starting AP
//          Start mDNS
// 10. If starting WiFi station
//          Start Wifi scan
//
// The rest of the startup is handled asynchronously by the WiFi
// event handler:
//
// For SCAN_COMPLETE
// 11. Select AP and channel
// 12. Set station host name
// 13. Disable auto reconnect
// 14. Connect to AP
//
// For STA_CONNECTED
// 15. Set stationConnected
//
// For GOT_IP or GOT_IP6
// 16. Set stationHasIp
// 17. Save IP address
// 18. Save IP address type
// 19. Display IP address
// 20. If AP mode not running
//      Start mDNS
//
// For LOST_IP
//  1. Clear stationHasIp
//  2. Display IP address
//
// In WiFi event handler for all other WiFi station events except
// STA_GOT_IP, STA_GOT_IP6 or STA_LOST_IP
//  1. If ESP-NOW and soft AP not running
//      Clear WiFi channel
//  2. Clear stationConnected
//  3. Clear stationHasIp
//  4. For STA_STOP
//      A. Clear timer
//     Else for STA_DISCONNECTED
//      A. Start timer (set to non-zero value)
//
// In wifiUpdate
//  5. When timer fires
//      A. Start scan
//----------------------------------------------------------------------

// Synchronous operations
// Common operations
#define WIFI_START_EVENT_HANDLER                 1

// Radio operations
#define WIFI_AP_SET_MODE                         2
#define WIFI_EN_SET_MODE                         4
#define WIFI_STA_SET_MODE                        8
#define WIFI_AP_SET_PROTOCOLS           0x00000010
#define WIFI_EN_SET_PROTOCOLS           0x00000020
#define WIFI_STA_SET_PROTOCOLS          0x00000040
#define WIFI_STA_START_SCAN             0x00000080
#define WIFI_STA_SELECT_REMOTE_AP       0x00000100
#define WIFI_AP_SELECT_CHANNEL          0x00000200
#define WIFI_EN_SELECT_CHANNEL          0x00000400
#define WIFI_STA_SELECT_CHANNEL         0x00000800

// Soft AP
#define WIFI_AP_SET_SSID_PASSWORD       0x00001000
#define WIFI_AP_SET_IP_ADDR             0x00002000
#define WIFI_AP_SET_HOST_NAME           0x00004000
#define WIFI_AP_START_MDNS              0x00008000
#define WIFI_AP_ONLINE                  0x00010000

// WiFi station
#define WIFI_STA_SET_HOST_NAME          0x00020000
#define WIFI_STA_DISABLE_AUTO_RECONNECT 0x00040000
#define WIFI_STA_CONNECT_TO_REMOTE_AP   0x00080000
#define WIFI_STA_START_MDNS             0x00100000
#define WIFI_STA_ONLINE                 0x00200000

// ESP-NOW
#define WIFI_EN_SET_CHANNEL             0x00400000
#define WIFI_EN_SET_PROMISCUOUS_MODE    0x00800000
#define WIFI_EN_PROMISCUOUS_RX_CALLBACK 0x01000000
#define WIFI_EN_START_ESP_NOW           0x02000000
#define WIFI_EN_ESP_NOW_ONLINE          0x04000000

// WIFI_MAX_START must be the last value in the define list
#define WIFI_MAX_START                  0x08000000

const char * const wifiStartNames[] =
{
    "WIFI_START_EVENT_HANDLER",

    "WIFI_AP_SET_MODE",
    "WIFI_EN_SET_MODE",
    "WIFI_STA_SET_MODE",
    "WIFI_AP_SET_PROTOCOLS",
    "WIFI_EN_SET_PROTOCOLS",
    "WIFI_STA_SET_PROTOCOLS",
    "WIFI_STA_START_SCAN",
    "WIFI_STA_SELECT_REMOTE_AP",
    "WIFI_AP_SELECT_CHANNEL",
    "WIFI_EN_SELECT_CHANNEL",
    "WIFI_STA_SELECT_CHANNEL",

    "WIFI_AP_SET_SSID_PASSWORD",
    "WIFI_AP_SET_IP_ADDR",
    "WIFI_AP_SET_HOST_NAME",
    "WIFI_AP_START_MDNS",
    "WIFI_AP_ONLINE",

    "WIFI_STA_SET_HOST_NAME",
    "WIFI_STA_DISABLE_AUTO_RECONNECT",
    "WIFI_STA_CONNECT_TO_REMOTE_AP",
    "WIFI_STA_START_MDNS",
    "WIFI_STA_ONLINE",

    "WIFI_EN_SET_CHANNEL",
    "WIFI_EN_SET_PROMISCUOUS_MODE",
    "WIFI_EN_PROMISCUOUS_RX_CALLBACK",
    "WIFI_EN_START_ESP_NOW",
    "WIFI_EN_ESP_NOW_ONLINE",
};
const int wifiStartNamesEntries = sizeof(wifiStartNames) / sizeof(wifiStartNames[0]);

#define WIFI_START_ESP_NOW          (WIFI_START_EVENT_HANDLER           \
                                     | WIFI_EN_SET_MODE                 \
                                     | WIFI_EN_SET_PROTOCOLS            \
                                     | WIFI_EN_SELECT_CHANNEL           \
                                     | WIFI_EN_SET_CHANNEL              \
                                     | WIFI_EN_PROMISCUOUS_RX_CALLBACK  \
                                     | WIFI_EN_SET_PROMISCUOUS_MODE     \
                                     | WIFI_EN_START_ESP_NOW            \
                                     | WIFI_EN_ESP_NOW_ONLINE)

#define WIFI_START_SOFT_AP          (WIFI_START_EVENT_HANDLER       \
                                     | WIFI_AP_SET_MODE             \
                                     | WIFI_AP_SET_PROTOCOLS        \
                                     | WIFI_AP_SELECT_CHANNEL       \
                                     | WIFI_AP_SET_SSID_PASSWORD    \
                                     | WIFI_AP_SET_IP_ADDR          \
                                     | WIFI_AP_SET_HOST_NAME        \
                                     | WIFI_AP_START_MDNS           \
                                     | WIFI_AP_ONLINE)

#define WIFI_START_STATION          (WIFI_START_EVENT_HANDLER           \
                                     | WIFI_STA_SET_MODE                \
                                     | WIFI_STA_SET_PROTOCOLS           \
                                     | WIFI_STA_START_SCAN              \
                                     | WIFI_STA_SELECT_CHANNEL          \
                                     | WIFI_STA_SELECT_REMOTE_AP        \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_STA_START_MDNS              \
                                     | WIFI_STA_ONLINE)

#define WIFI_STA_RECONNECT          (WIFI_STA_START_SCAN                \
                                     | WIFI_STA_SELECT_CHANNEL          \
                                     | WIFI_STA_SELECT_REMOTE_AP        \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_STA_START_MDNS              \
                                     | WIFI_STA_ONLINE)

#define WIFI_SELECT_CHANNEL         (WIFI_AP_SELECT_CHANNEL     \
                                     | WIFI_EN_SELECT_CHANNEL   \
                                     | WIFI_STA_SELECT_CHANNEL)

#define WIFI_STA_NO_REMOTE_AP       (WIFI_STA_SELECT_CHANNEL            \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_STA_START_MDNS              \
                                     | WIFI_STA_ONLINE)

#define WIFI_STA_FAILED_SCAN        (WIFI_STA_START_SCAN          \
                                     | WIFI_STA_SELECT_REMOTE_AP  \
                                     | WIFI_STA_NO_REMOTE_AP)

//****************************************
// WiFi SSIDs and Passwords
//****************************************

// Entry in the SSID and password table wifiSsidPassword
typedef struct _SSID_PASSWORD_t
{
    const char * ssid;      // ID of access point
    const char * password;  // Password for the access point
} SSID_PASSWORD_t;

const SSID_PASSWORD_t wifiSsidPassword[] =
{
    {wifiSSID1, wifiPassword1},
    {wifiSSID2, wifiPassword2},
    {wifiSSID3, wifiPassword3},
    {wifiSSID4, wifiPassword4},
};
const int wifiSsidPasswordEntries = sizeof(wifiSsidPassword)
                                  / sizeof(wifiSsidPassword[0]);

//*********************************************************************
// Callback for all WiFi RX Packets
// Get RSSI of all incoming management packets: https://esp32.com/viewtopic.php?t=13889
void wifiPromiscuousRxHandler(void *buf, wifi_promiscuous_pkt_type_t type)
{
    const wifi_promiscuous_pkt_t *ppkt; // Defined in esp_wifi_types_native.h

    // All espnow traffic uses action frames which are a subtype of the
    // mgmnt frames so filter out everything else.
    if (type != WIFI_PKT_MGMT)
        return;

    ppkt = (wifi_promiscuous_pkt_t *)buf;
    packetRSSI = ppkt->rx_ctrl.rssi;
}

//*********************************************************************
// Constructor
// Inputs:
//   display: Set to true to display WiFi startup summary
//   debug: Set to true to display WiFi debugging messages
RTK_WIFI::RTK_WIFI(bool display, bool debug)
    : _apChannel{0}, _apCount{0}, _apDnsAddress{IPAddress((uint32_t)0)},
      _apFirstDhcpAddress{IPAddress("192.168.4.32")},
      _apGatewayAddress{(uint32_t)0},
      _apIpAddress{IPAddress("192.168.4.1")},
      _apMacAddress{0, 0, 0, 0, 0, 0},
      _apSubnetMask{IPAddress("255.255.255.0")}, _channel{0},
      _debug{debug}, _display{display},
      _espNowChannel{0}, _espNowStarted{false},
      _hostName{nullptr}, _scanRunning{false},
      _staIpAddress{IPAddress((uint32_t)0)}, _staIpType{0},
      _staMacAddress{0, 0, 0, 0, 0, 0},
      _staRemoteApSsid{nullptr}, _staRemoteApPassword{nullptr},
      _started{false}, _stationChannel{0},
      _timer{0}, _verbose{0}
{
}

//*********************************************************************
// Enable or disable WiFi debug
// Inputs:
//   enable: Set true to enable debug
// Outputs:
//   Return the previous enable value
bool RTK_WIFI::debug(bool enable)
{
    bool oldDebug;

    oldDebug = _debug;
    _debug = enable;
    if (enable)
        display(enable);
    return oldDebug;
}

//*********************************************************************
// Enable or disable WiFi display
// Inputs:
//   enable: Set true to enable display
// Outputs:
//   Return the previous enable value
bool RTK_WIFI::display(bool enable)
{
    bool oldDisplay;

    oldDisplay = _display;
    _display = enable;
    return oldDisplay;
}

//*********************************************************************
// Display components begin started or stopped
// Inputs:
//   text: Text describing the component list
//   components: A bit mask of the components
void RTK_WIFI::displayComponents(const char * text, WIFI_ACTION_t components)
{
    WIFI_ACTION_t mask;

    systemPrintf("%s: 0x%08x\r\n", text, components);
    for (int index = wifiStartNamesEntries - 1; index >= 0; index--)
    {
        mask = 1 << index;
        if (components & mask)
            systemPrintf("    0x%08lx: %s\r\n", mask, wifiStartNames[index]);
    }
}

//*********************************************************************
// Enable or disable the WiFi modes
// Inputs:
//   enableESPNow: Enable ESP-NOW mode
//   enableSoftAP: Enable soft AP mode
//   enableStataion: Enable station mode
// Outputs:
//   Returns true if the modes were successfully configured
bool RTK_WIFI::enable(bool enableESPNow, bool enableSoftAP, bool enableStation)
{
    int authIndex;
    WIFI_ACTION_t starting;
    WIFI_ACTION_t stopping;

    // Determine the next actions
    starting = 0;
    stopping = 0;

    // Display the parameters
    if (_debug && _verbose)
    {
        systemPrintf("enableESPNow: %s\r\n", enableESPNow ? "true" : "false");
        systemPrintf("enableSoftAP: %s\r\n", enableSoftAP ? "true" : "false");
        systemPrintf("enableStation: %s\r\n", enableStation ? "true" : "false");
    }

    // Update the ESP-NOW state
    if (enableESPNow)
        starting |= WIFI_START_ESP_NOW;
    else
        stopping |= WIFI_START_ESP_NOW;

    // Update the soft AP state
    if (enableSoftAP)
    {
        // Verify that the SSID is set
        if (wifiApSsid && strlen(wifiApSsid) && wifiApPassword)
            starting |= WIFI_START_SOFT_AP;
        else
            systemPrintf("ERROR: AP SSID or password is missing\r\n");
    }
    else
        stopping |= WIFI_START_SOFT_AP;

    // Update the station state
    if (enableStation)
    {
        // Verify that at least one WiFi access point is in the list
        if (wifiSsidPasswordEntries == 0)
            systemPrintf("ERROR: No entries in wiFiSsidPassword\r\n");
        else
        {
            // Verify that at least one SSID is set
            for (authIndex = 0; authIndex < wifiSsidPasswordEntries; authIndex++)
                if (wifiSsidPassword[authIndex].ssid
                    && (strlen(wifiSsidPassword[authIndex].ssid)))
                {
                    break;
                }
            if (authIndex >= wifiSsidPasswordEntries)
                systemPrintf("ERROR: No valid SSID in wifiSsidPassword\r\n");
            else
            {
                // Start the WiFi station
                starting |= WIFI_START_STATION;
                _stationRunning = true;
            }
        }
    }
    else
    {
        // Stop the WiFi station
        stopping |= WIFI_START_STATION;
        _stationRunning = false;
    }

    // Stop and start the WiFi components
    return stopStart(stopping, starting);
}

//*********************************************************************
// Handle the WiFi event
void wifiEventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    wifi.eventHandler(event, info);
}

//*********************************************************************
// Handle the WiFi event
void RTK_WIFI::eventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    bool success;

    if (_debug)
        systemPrintf("event: %d (%s)\r\n", event, arduinoEventName[event]);

    // Handle the event
    switch (event)
    {

    //------------------------------
    // Controller events
    //------------------------------

    case ARDUINO_EVENT_WIFI_OFF:
    case ARDUINO_EVENT_WIFI_READY:
        break;

    //----------------------------------------
    // Scan events
    //----------------------------------------

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        stationEventHandler(event, info);
        break;

    //------------------------------
    // Station events
    //------------------------------
    case ARDUINO_EVENT_WIFI_STA_START:
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    case ARDUINO_EVENT_WIFI_STA_STOP:
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        stationEventHandler(event, info);
        break;

    //----------------------------------------
    // Soft AP events
    //----------------------------------------

    case ARDUINO_EVENT_WIFI_AP_START:
    case ARDUINO_EVENT_WIFI_AP_STOP:
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        softApEventHandler(event, info);
        break;
    }
}

//*********************************************************************
// Start the WiFi event handler
void RTK_WIFI::eventHandlerStart()
{
    if (_debug)
        systemPrintf("Starting the WiFi event handler\r\n");

    // Establish the WiFi event handler
    Network.onEvent(wifiEventHandler);
}

//*********************************************************************
// Stop the WiFi event handler
void RTK_WIFI::eventHandlerStop()
{
    if (_debug)
        systemPrintf("Stopping the WiFi event handler\r\n");

    Network.removeEvent(wifiEventHandler);
}

//*********************************************************************
// Get the mDNS host name
const char * RTK_WIFI::hostNameGet()
{
    return _hostName;
}

//*********************************************************************
// Get the mDNS host name
void RTK_WIFI::hostNameSet(const char * mDnsHostName)
{
    _hostName = mDnsHostName;
}

//*********************************************************************
// Start the multicast domain name server
// Inputs:
//   softAp: True when starting mDNS on the soft AP
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::mDNSStart(bool softAp)
{
    bool started;
    IPAddress * ipAddress;

    do
    {
        started = true;
        if (_hostName)
        {
            // Verify that a host name exists
            started = (strlen(_hostName) != 0);
            if (!started)
            {
                systemPrintf("ERROR: No mDNS host name specified!\r\n");
                break;
            }

            // Start mDNS
            if (_debug)
                systemPrintf("Starting mDNS on %s\r\n", softAp ? "soft AP" : "WiFi station");
            started = MDNS.begin(_hostName);
            if (!started)
            {
                systemPrintf("ERROR: Failed to start mDNS for %s!\r\n",
                             softAp ? "soft AP" : "WiFi station");
                break;
            }
            if (_debug)
            {
                ipAddress = softAp ? &_apIpAddress : &_staIpAddress;
                systemPrintf("mDNS started on %s as %s.local (%s)\r\n",
                             softAp ? "soft AP" : "WiFi station",
                             _hostName,
                             ipAddress->toString().c_str());
            }
        }
    } while (0);
    return started;
}

//*********************************************************************
// Stop the multicast domain name server
void RTK_WIFI::mDNSStop()
{
    if (_hostName)
    {
        if (_debug)
            systemPrintf("Stopping mDNS\r\n");
        MDNS.end();
        if (_debug)
            systemPrintf("mDNS stopped\r\n");
    }
}

//*********************************************************************
// Set the WiFi mode
// Inputs:
//   setMode: Modes to set
//   xorMode: Modes to toggle
//
// Math: result = (mode | setMode) ^ xorMode
//
//                              setMode
//                      0                   1
//  xorMode 0       No change           Set bit
//          1       Toggle bit          Clear bit
//
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::setWiFiMode(uint8_t setMode, uint8_t xorMode)
{
    uint8_t mode;
    uint8_t newMode;
    bool started;
    esp_err_t status;

    started = false;
    do
    {
        // Get the current mode
        mode = (uint8_t)WiFi.getMode();
        if (_debug && _verbose)
            systemPrintf("Current WiFi mode: 0x%08x (%s)\r\n",
                         mode,
                         ((mode == 0) ? "WiFi off"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP"
                         : "STA"))));

        // Determine the new mode
        newMode = (mode | setMode) ^ xorMode;
        started = (newMode == mode);
        if (started)
            break;

        // Set the new mode
        started = WiFi.mode((wifi_mode_t)newMode);
        if (!started)
        {
            systemPrintf("ERROR: Failed to set %d (%s), status: %d!\r\n",
                         newMode,
                         ((newMode == 0) ? "WiFi off"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA mode"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP mode"
                         : "STA mode"))), status);
            break;
        }
        if (_debug && _verbose)
            systemPrintf("Set WiFi: %d (%s)\r\n",
                         newMode,
                         ((newMode == 0) ? "Off"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA mode"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP mode"
                         : "STA mode"))));
    } while (0);

    // Return the final status
    return started;
}

//*********************************************************************
// Set the WiFi radio protocols
// Inputs:
//   interface: Interface on which to set the protocols
//   enableWiFiProtocols: When true, enable the WiFi protocols
//   enableLongRangeProtocol: When true, enable the long range protocol
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::setWiFiProtocols(wifi_interface_t interface,
                                bool enableWiFiProtocols,
                                bool enableLongRangeProtocol)
{
    uint8_t newProtocols;
    uint8_t oldProtocols;
    bool started;
    esp_err_t status;

    started = false;
    do
    {
        // Get the current protocols
        status = esp_wifi_get_protocol(interface, &oldProtocols);
        started = (status == ESP_OK);
        if (!started)
        {
            systemPrintf("ERROR: Failed to get the WiFi %s radio protocols!\r\n",
                         (interface == WIFI_IF_AP) ? "soft AP" : "station");
            break;
        }
        if (_debug && _verbose)
            systemPrintf("Current WiFi protocols (%d%s%s%s%s%s)\r\n",
                         oldProtocols,
                         oldProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         oldProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         oldProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         oldProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         oldProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");

        // Determine which protocols to enable
        newProtocols = oldProtocols;
        if (enableLongRangeProtocol || enableWiFiProtocols)
        {
            // Enable the WiFi protocols
            newProtocols |= WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;

            // Enable the ESP-NOW long range protocol
            if (enableLongRangeProtocol)
                newProtocols |= WIFI_PROTOCOL_LR;
            else
                newProtocols &= ~WIFI_PROTOCOL_LR;
        }

        // Disable the protocols
        else
            newProtocols = 0;

        // Display the new protocols
        if (_debug && _verbose)
            systemPrintf("Setting WiFi protocols (%d%s%s%s%s%s)\r\n",
                         newProtocols,
                         newProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         newProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         newProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         newProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         newProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");

        // Set the new protocols
        started = true;
        if (newProtocols != oldProtocols)
        {
            status = esp_wifi_set_protocol(interface, newProtocols);
            started = (status == ESP_OK);
        }
        if (!started)
        {
            systemPrintf("Current WiFi protocols (%d%s%s%s%s%s)\r\n",
                         oldProtocols,
                         oldProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         oldProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         oldProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         oldProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         oldProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");
            systemPrintf("Setting WiFi protocols (%d%s%s%s%s%s)\r\n",
                         newProtocols,
                         newProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         newProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         newProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         newProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         newProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");
            systemPrintf("ERROR: Failed to set the WiFi %s radio protocols!\r\n",
                         (interface == WIFI_IF_AP) ? "soft AP" : "station");
            break;
        }
    } while (0);

    // Return the final status
    return started;
}

//*********************************************************************
// Configure the soft AP
// Inputs:
//   ipAddress: IP address of the soft AP
//   subnetMask: Subnet mask for the soft AP network
//   firstDhcpAddress: First IP address to use in the DHCP range
//   dnsAddress: IP address to use for DNS lookup (translate name to IP address)
//   gatewayAddress: IP address of the gateway to a larger network (internet?)
// Outputs:
//   Returns true if the soft AP was successfully configured.
bool RTK_WIFI::softApConfiguration(IPAddress ipAddress,
                                   IPAddress subnetMask,
                                   IPAddress firstDhcpAddress,
                                   IPAddress dnsAddress,
                                   IPAddress gatewayAddress)
{
    bool success;

    _apIpAddress = ipAddress;
    _apSubnetMask = subnetMask;
    _apFirstDhcpAddress = firstDhcpAddress;
    _apDnsAddress = dnsAddress;
    _apGatewayAddress = gatewayAddress;

    // Restart the soft AP if necessary
    success = true;
    if (softApOnline())
    {
        success = enable(false, false, stationRunning());
        if (success)
            success = enable(false, true, stationRunning());
    }
    return success;
}

//*********************************************************************
// Display the soft AP configuration
// Inputs:
//   display: Address of a Print object
void RTK_WIFI::softApConfigurationDisplay(Print * display)
{
    display->printf("Soft AP configuration:\r\n");
    display->printf("    %s: IP Address\r\n", _apIpAddress.toString().c_str());
    display->printf("    %s: Subnet mask\r\n", _apSubnetMask.toString().c_str());
    if ((uint32_t)_apFirstDhcpAddress)
        display->printf("    %s: First DHCP address\r\n", _apFirstDhcpAddress.toString().c_str());
    if ((uint32_t)_apDnsAddress)
        display->printf("    %s: DNS address\r\n", _apDnsAddress.toString().c_str());
    if ((uint32_t)_apGatewayAddress)
        display->printf("    %s: Gateway address\r\n", _apGatewayAddress.toString().c_str());
}

//*********************************************************************
// Handle the soft AP events
void RTK_WIFI::softApEventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    // Handle the event
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_STOP:
        // Mark the soft AP as offline
        if (_display && softApOnline())
            systemPrintf("AP: Offline\r\n");
        _started = _started & ~WIFI_AP_ONLINE;
        if (_debug && _verbose)
            systemPrintf("_started: 0x%08x\r\n", _started);
        break;
    }
}

//*********************************************************************
// Set the soft AP host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::softApSetHostName(const char * hostName)
{
    bool nameSet;

    do
    {
        // Verify that a host name was specified
        nameSet =  (hostName != nullptr) && (strlen(hostName) != 0);
        if (!nameSet)
        {
            systemPrintf("ERROR: No host name specified!\r\n");
            break;
        }

        // Set the host name
        if (_debug)
            systemPrintf("WiFI setting AP host name\r\n");
        nameSet = WiFi.AP.setHostname(hostName);
        if (!nameSet)
        {
            systemPrintf("ERROR: Failed to set the Wifi AP host name!\r\n");
            break;
        }
        if (_debug)
            systemPrintf("WiFi AP hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Set the soft AP configuration
// Inputs:
//   ipAddress: IP address of the server, nullptr or empty string causes
//              default 192.168.4.1 to be used
//   subnetMask: Subnet mask for local network segment, nullptr or empty
//              string causes default 0.0.0.0 to be used, unless ipAddress
//              is not specified, in which case 255.255.255.0 is used
//   gatewayAddress: Gateway to internet IP address, nullptr or empty string
//            causes default 0.0.0.0 to be used (no access to internet)
//   dnsAddress: Domain name server (name to IP address translation) IP address,
//              nullptr or empty string causes 0.0.0.0 to be used (only
//              mDNS name translation, if started)
//   dhcpStartAddress: Start of DHCP IP address assignments for the local
//              network segment, nullptr or empty string causes default
//              0.0.0.0 to be used (disable DHCP server)  unless ipAddress
//              was not specified in which case 192.168.4.2
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::softApSetIpAddress(const char * ipAddress,
                                  const char * subnetMask,
                                  const char * gatewayAddress,
                                  const char * dnsAddress,
                                  const char * dhcpFirstAddress)
{
    bool configured;
    uint32_t uDhcpFirstAddress;
    uint32_t uDnsAddress;
    uint32_t uGatewayAddress;
    uint32_t uIpAddress;
    uint32_t uSubnetMask;

    // Convert the IP address
    if ((!ipAddress) || (strlen(ipAddress) == 0))
        uIpAddress = 0;
    else
        uIpAddress = (uint32_t)IPAddress(ipAddress);

    // Convert the subnet mask
    if ((!subnetMask) || (strlen(subnetMask) == 0))
    {
        if (uIpAddress == 0)
            uSubnetMask = IPAddress("255.255.255.0");
        else
            uSubnetMask = 0;
    }
    else
        uSubnetMask = (uint32_t)IPAddress(subnetMask);

    // Convert the gateway address
    if ((!gatewayAddress) || (strlen(gatewayAddress) == 0))
        uGatewayAddress = 0;
    else
        uGatewayAddress = (uint32_t)IPAddress(gatewayAddress);

    // Convert the first DHCP address
    if ((!dhcpFirstAddress) || (strlen(dhcpFirstAddress) == 0))
    {
        if (uIpAddress == 0)
            uDhcpFirstAddress = IPAddress("192.168.4.32");
        else
            uDhcpFirstAddress = uIpAddress + 0x1f000000;
    }
    else
        uDhcpFirstAddress = (uint32_t)IPAddress(dhcpFirstAddress);

    // Convert the DNS address
    if ((!dnsAddress) || (strlen(dnsAddress) == 0))
        uDnsAddress = 0;
    else
        uDnsAddress = (uint32_t)IPAddress(dnsAddress);

    // Use the default IP address if not specified
    if (uIpAddress == 0)
        uIpAddress = IPAddress("192.168.4.1");

    // Display the soft AP configuration
    if (_debug)
        softApConfigurationDisplay(&Serial);

    // Configure the soft AP
    configured = WiFi.AP.config(IPAddress(uIpAddress),
                                IPAddress(uGatewayAddress),
                                IPAddress(uSubnetMask),
                                IPAddress(uDhcpFirstAddress),
                                IPAddress(uDnsAddress));
    if (!configured)
        systemPrintf("ERROR: Failed to configure the soft AP with IP addresses!\r\n");
    return configured;
}

//*********************************************************************
// Get the soft AP status
bool RTK_WIFI::softApOnline()
{
    return (_started & WIFI_AP_ONLINE) ? true : false;
}

//*********************************************************************
// Set the soft AP SSID and password
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::softApSetSsidPassword(const char * ssid, const char * password)
{
    bool created;

    // Set the WiFi soft AP SSID and password
    if (_debug)
        systemPrintf("WiFi AP: Attempting to set AP SSID and password\r\n");
    created = WiFi.AP.create(ssid, password);
    if (!created)
        systemPrintf("ERROR: Failed to set soft AP SSID and Password!\r\n");
    else if (_debug)
        systemPrintf("WiFi AP: SSID: %s, Password: %s\r\n", ssid, password);
    return created;
}

//*********************************************************************
// Connect the station to a remote AP
// Return true if the connection was successful and false upon failure.
bool RTK_WIFI::stationConnectAP()
{
    bool connected;

    do
    {
        // Connect to the remote AP
        if (_debug)
            systemPrintf("WiFi connecting to %s on channel %d with %s authorization\r\n",
                         _staRemoteApSsid,
                         _channel,
                         (_staAuthType < WIFI_AUTH_MAX) ? wifiAuthorizationName[_staAuthType] : "Unknown");
        connected = (WiFi.STA.connect(_staRemoteApSsid, _staRemoteApPassword, _channel));
        if (!connected)
        {
            if (_debug)
                systemPrintf("WIFI failed to connect to SSID %s with password %s\r\n",
                             _staRemoteApSsid, _staRemoteApPassword);
            break;
        }
        if (_debug)
            systemPrintf("WiFi station connected to %s on channel %d with %s authorization\r\n",
                         _staRemoteApSsid,
                         _channel,
                         (_staAuthType < WIFI_AUTH_MAX) ? wifiAuthorizationName[_staAuthType] : "Unknown");
    } while (0);
    return connected;
}

//*********************************************************************
// Disconnect the station from an AP
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::stationDisconnect()
{
    bool disconnected;

    do
    {
        // Determine if station is connected to a remote AP
        disconnected = !_staConnected;
        if (disconnected)
        {
            if (_debug)
                systemPrintf("Station already disconnected from remote AP\r\n");
            break;
        }

        // Disconnect from the remote AP
        if (_debug)
            systemPrintf("WiFI disconnecting station from remote AP\r\n");
        disconnected = WiFi.STA.disconnect();
        if (!disconnected)
        {
            systemPrintf("ERROR: Failed to disconnect WiFi from the remote AP!\r\n");
            break;
        }
        if (_debug)
            systemPrintf("WiFi disconnected from the remote AP\r\n");
    } while (0);
    return disconnected;
}

//*********************************************************************
// Handle the WiFi station events
void RTK_WIFI::stationEventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    WIFI_CHANNEL_t channel;
    IPAddress ipAddress;
    bool success;
    int type;

    // Notify the upper layers that WiFi is no longer available
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_START:
        WiFi.STA.macAddress(_staMacAddress);
        if (_debug)
            systemPrintf("WiFi Event: Station start: MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                         _staMacAddress[0], _staMacAddress[1], _staMacAddress[2],
                         _staMacAddress[3], _staMacAddress[4], _staMacAddress[5]);

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_STA_STOP:
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        // Start the reconnection timer
        if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
        {
            if (_debug && _verbose && !_timer)
                systemPrintf("Reconnection timer started\r\n");
            _timer = millis();
            if (!_timer)
                _timer = 1;
        }
        else
        {
            // Stop the reconnection timer
            if (_debug && _verbose && _timer)
                systemPrintf("Reconnection timer stopped\r\n");
            _timer = 0;
        }

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        // The WiFi station is no longer connected to the remote AP
        if (_debug && _staConnected)
            systemPrintf("WiFi station disconnected from %s\r\n",
                         _staRemoteApSsid);
        _staConnected = false;

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        // Mark the WiFi station offline
        if (_display && (_started & WIFI_STA_ONLINE))
            systemPrintf("WiFi: Station offline!\r\n");
        _started = _started & ~WIFI_STA_ONLINE;

        // Notify user of loss of IP address
        if (_debug && _staHasIp)
            systemPrintf("WiFi station lost IPv%c address %s\r\n",
                         _staIpType, _staIpAddress.toString().c_str());
        _staHasIp = false;

        // Stop mDNS if necessary
        if (_started & WIFI_STA_START_MDNS)
        {
            if (_debug && _verbose)
                systemPrintf("Calling mDNSStop for WiFi station from %s event\r\n",
                             arduinoEventName[event]);
            _started = _started & ~WIFI_STA_START_MDNS;
            mDNSStop();
        }
        _staIpAddress = IPAddress((uint32_t)0);
        _staIpType = 0;
        break;

    //------------------------------
    // WiFi Station State Machine
    //
    //   .--------+<----------+<-----------+<-------------+<----------+<----------+<------------.
    //   v        |           |            |              |           |           |             |
    // STOP --> READY --> STA_START --> SCAN_DONE --> CONNECTED --> GOT_IP --> LOST_IP --> DISCONNECTED
    //            ^                                       ^           ^           |             |
    //            |                                       |           '-----------'             |
    // OFF -------'                                       '-------------------------------------'
    //
    // Handle the WiFi station events
    //------------------------------

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        _staConnected = true;
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        ipAddress = WiFi.STA.localIP();
        type = (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) ? '4' : '6';

        // Display the IP address
        if (_debug)
            systemPrintf("WiFi Event: Got IPv%c address %s\r\n",
                         type, ipAddress.toString().c_str());
        break;
    }   // End of switch
}

//*********************************************************************
// Set the station's host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool RTK_WIFI::stationHostName(const char * hostName)
{
    bool nameSet;

    do
    {
        // Verify that a host name was specified
        nameSet =  (hostName != nullptr) && (strlen(hostName) != 0);
        if (!nameSet)
        {
            systemPrintf("ERROR: No host name specified!\r\n");
            break;
        }

        // Set the host name
        if (_debug)
            systemPrintf("WiFI setting station host name\r\n");
        nameSet = WiFi.STA.setHostname(hostName);
        if (!nameSet)
        {
            systemPrintf("ERROR: Failed to set the Wifi station host name!\r\n");
            break;
        }
        if (_debug)
            systemPrintf("WiFi station hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Get the station status
bool RTK_WIFI::stationOnline()
{
    return (_started & WIFI_STA_ONLINE) ? true : false;
}

//*********************************************************************
// Handle WiFi station reconnection requests
void RTK_WIFI::stationReconnectionRequest()
{
    uint32_t currentMsec;

    // Check for reconnection request
    currentMsec = millis();
    if (_timer)
    {
        if ((currentMsec - _timer) >= WIFI_RECONNECTION_DELAY)
        {
            _timer = 0;
            if (_debug)
                systemPrintf("Reconnection timer fired!\r\n");

            // Start the WiFi scan
            if (stationRunning())
            {
                _started = _started & ~WIFI_STA_RECONNECT;
                stopStart(WIFI_AP_START_MDNS, WIFI_STA_RECONNECT);
            }
        }
    }
}

//*********************************************************************
// Scan the WiFi network for remote APs
// Inputs:
//   channel: Channel number for the scan, zero (0) scan all channels
// Outputs:
//   Returns the number of access points
int16_t RTK_WIFI::stationScanForAPs(WIFI_CHANNEL_t channel)
{
    int16_t apCount;
    int16_t status;

    do
    {
        // Determine if the WiFi scan is already running
        if (_scanRunning)
        {
            if (_debug)
                systemPrintf("WiFi scan already running");
            break;
        }

        // Determine if scanning a single channel or all channels
        if (_debug)
        {
            if (channel)
                systemPrintf("WiFI starting scan for remote APs on channel %d\r\n", channel);
            else
                systemPrintf("WiFI starting scan for remote APs\r\n");
        }

        // Start the WiFi scan
        apCount = WiFi.scanNetworks(false,      // async
                                    false,      // show_hidden
                                    false,      // passive
                                    300,        // max_ms_per_chan
                                    channel,    // channel number
                                    nullptr,    // ssid *
                                    nullptr);   // bssid *
        if (_debug && _verbose)
            Serial.printf("apCount: %d\r\n", apCount);
        if (apCount < 0)
        {
            systemPrintf("ERROR: WiFi scan failed, status: %d!\r\n", apCount);
            break;
        }
        _apCount = apCount;
        if (_debug)
            systemPrintf("WiFi scan complete, found %d remote APs\r\n", _apCount);
    } while (0);
    return apCount;
}

//*********************************************************************
// Get the station status
bool RTK_WIFI::stationRunning()
{
    return _stationRunning;
}

//*********************************************************************
// Select the AP and channel to use for WiFi station
// Inputs:
//   apCount: Number to APs detected by the WiFi scan
//   list: Determine if the APs should be listed
// Outputs:
//   Returns the channel number of the AP
WIFI_CHANNEL_t RTK_WIFI::stationSelectAP(uint8_t apCount, bool list)
{
    int ap;
    WIFI_CHANNEL_t apChannel;
    bool apFound;
    int authIndex;
    WIFI_CHANNEL_t channel;
    const char * ssid;
    String ssidString;
    int type;

    // Verify that an AP was found
    if (apCount == 0)
        return 0;

    // Print the header
    //                                    1                 1         2         3
    //             1234   1234   123456789012345   12345678901234567890123456789012
    if (_debug || list)
    {
        systemPrintf(" dBm   Chan   Authorization     SSID\r\n");
        systemPrintf("----   ----   ---------------   --------------------------------\r\n");
    }

    // Walk the list of APs that were found during the scan
    apFound = false;
    apChannel = 0;
    for (ap = 0; ap < apCount; ap++)
    {
        // The APs are listed in decending signal strength order
        // Check for a requested AP
        ssidString = WiFi.SSID(ap);
        ssid = ssidString.c_str();
        type = WiFi.encryptionType(ap);
        channel = WiFi.channel(ap);
        if (!apFound)
        {
            for (authIndex = 0; authIndex < wifiSsidPasswordEntries; authIndex++)
            {
                // Determine if this authorization matches the AP's SSID
                if (wifiSsidPassword[authIndex].ssid
                    && strlen(wifiSsidPassword[authIndex].ssid)
                    && (strcmp(ssid, wifiSsidPassword[authIndex].ssid) == 0)
                    && ((type == WIFI_AUTH_OPEN)
                        || (wifiSsidPassword[authIndex].password
                            && (strlen(wifiSsidPassword[authIndex].password)))))
                {
                    if (_debug)
                        systemPrintf("WiFi: Found remote AP: %s\r\n", ssid);

                    // A match was found, save it and stop looking
                    _staRemoteApSsid = wifiSsidPassword[authIndex].ssid;
                    _staRemoteApPassword = wifiSsidPassword[authIndex].password;
                    apChannel = channel;
                    _staAuthType = type;
                    apFound = true;
                    break;
                }
            }

            // Check for done
            if (apFound && (!(_debug | list)))
                break;
        }

        // Display the list of APs
        if (_debug || list)
            systemPrintf("%4d   %4d   %s   %s\r\n",
                         WiFi.RSSI(ap),
                         channel,
                         (type < WIFI_AUTH_MAX) ? wifiAuthorizationName[type] : "Unknown",
                         ssid);
    }

    // Return the channel number
    return apChannel;
}

//*********************************************************************
// Stop and start WiFi components
// Inputs:
//   stopping: WiFi components that need to be stopped
//   starting: WiFi components that neet to be started
// Outputs:
//   Returns true if the modes were successfully configured
bool RTK_WIFI::stopStart(WIFI_ACTION_t stopping, WIFI_ACTION_t starting)
{
    int authIndex;
    WIFI_CHANNEL_t channel;
    WIFI_ACTION_t delta;
    bool enabled;
    WIFI_ACTION_t mask;
    WIFI_ACTION_t notStarted;
    uint8_t primaryChannel;
    WIFI_ACTION_t restarting;
    wifi_second_chan_t secondaryChannel;
    WIFI_ACTION_t startingNow;
    esp_err_t status;

    // Determine the next actions
    notStarted = 0;
    enabled = true;

    // Display the parameters
    if (_debug && _verbose)
    {
        systemPrintf("stopping: 0x%08x\r\n", stopping);
        systemPrintf("starting: 0x%08x\r\n", starting);
    }

    //****************************************
    // Select the channel
    //
    // The priority order for the channel is:
    //      1. Active channel
    //      2. _stationChannel
    //      3. Remote AP channel determined by scan
    //      4. _espNowChannel
    //      5. _apChannel
    //      6. Channel 1
    //****************************************

    // Determine if there is an active channel
    if (((_started & ~stopping) & (WIFI_AP_ONLINE | WIFI_EN_ESP_NOW_ONLINE | WIFI_STA_ONLINE))
        && _channel)
    {
        // Continue to use the active channel
        channel = _channel;
        if (_debug && _verbose)
            systemPrintf("channel: %d, active channel\r\n", channel);
    }

    // Use the station channel if specified
    else if (_stationChannel && (starting & WIFI_STA_ONLINE))
    {
        channel = _stationChannel;
        if (_debug && _verbose)
            systemPrintf("channel: %d, WiFi station channel\r\n", channel);
    }

    // Determine if a scan for remote APs is needed
    else if (starting & WIFI_STA_START_SCAN)
    {
        channel = 0;
        if (_debug && _verbose)
            systemPrintf("channel: Determine by remote AP scan\r\n");
    }

    // Determine if the ESP-NOW channel was specified
    else if (_espNowChannel & ((starting | _started) & WIFI_EN_ESP_NOW_ONLINE))
    {
        channel = _espNowChannel;
        if (_debug && _verbose)
            systemPrintf("channel: %d, ESP-NOW channel\r\n", channel);
    }

    // Determine if the AP channel was specified
    else if (_apChannel && ((starting | _started) & WIFI_AP_ONLINE))
    {
        channel = _apChannel;
        if (_debug && _verbose)
            systemPrintf("channel: %d, soft AP channel\r\n", channel);
    }

    // No channel specified and scan not being done, use the default channel
    else
    {
        channel = WIFI_DEFAULT_CHANNEL;
        if (_debug && _verbose)
            systemPrintf("channel: %d, default channel\r\n", channel);
    }

    //****************************************
    // Determine the use of mDNS
    //****************************************

    // It is much more difficult to determine the DHCP address of the RTK
    // versus the hard coded IP address of the server.  As such give
    // priority to the WiFi station for mDNS use.  When the station is
    // not running or being started, let mDNS start for the soft AP.

    // Determine if mDNS is being used for WiFi station
    if (starting & WIFI_STA_START_MDNS)
    {
        // Don't start mDNS for soft AP
        starting &= ~WIFI_AP_START_MDNS;

        // Stop it if it is being used for soft AP
        if (_started & WIFI_AP_START_MDNS)
            stopping |= WIFI_AP_START_MDNS;
    }

    //****************************************
    // Perform some optimizations
    //****************************************

    // Only stop the started components
    stopping &= _started;

    // Determine which components are being restarted
    restarting = _started & stopping & starting;
    if (_debug && _verbose)
    {
        systemPrintf("0x%08x: _started\r\n", _started);
        systemPrintf("0x%08x: stopping\r\n", stopping);
        systemPrintf("0x%08x: starting\r\n", starting);
        systemPrintf("0x%08x: restarting\r\n", starting);
    }

    // Only start or stop the WiFi event handler
    if (restarting & WIFI_START_EVENT_HANDLER)
    {
        stopping &= ~WIFI_START_EVENT_HANDLER;
        starting &= ~WIFI_START_EVENT_HANDLER;
    }

    // Don't start components that are already running and are not being
    // stopped
    starting &= ~(_started & ~stopping);

    //****************************************
    // Return the use of mDNS to soft AP when WiFi STA stops
    //****************************************

    // Determine if WiFi STA is stopping
    if (stopping & WIFI_STA_START_MDNS)
    {
        // Determine if mDNS should continue to run on soft AP
        if ((_started & WIFI_AP_ONLINE) && !(stopping & WIFI_AP_ONLINE))
        {
            // Restart mDNS for soft AP
            _started = _started & ~WIFI_AP_START_MDNS;
            starting |= WIFI_AP_START_MDNS;
        }
    }

    // Stop the components
    startingNow = starting;
    do
    {
        //****************************************
        // Display the items being stopped
        //****************************************

        if (_debug && _verbose && stopping)
            displayComponents("Stopping", stopping);

        //****************************************
        // Stop the ESP-NOW components
        //****************************************

        // Mark the ESP-NOW offline
        if (stopping & WIFI_EN_ESP_NOW_ONLINE)
        {
            if (_debug)
                systemPrintf("WiFi: ESP-NOW offline!\r\n");
            _started = _started & ~WIFI_EN_ESP_NOW_ONLINE;
        }

        // Stop the ESP-NOW layer
        if (stopping & WIFI_EN_START_ESP_NOW)
        {
            if (_debug && _verbose)
                systemPrintf("WiFi: Stopping ESP-NOW\r\n");
            if (!espNowStop())
            {
                systemPrintf("ERROR: Failed to stop ESP-NOW!\r\n");
                break;
            }
            _started = _started & ~WIFI_EN_START_ESP_NOW;
            if (_debug && _verbose)
                systemPrintf("WiFi: ESP-NOW stopped\r\n");
        }

        // Stop the promiscuous RX callback handler
        if (stopping & WIFI_EN_PROMISCUOUS_RX_CALLBACK)
        {
            if (_debug && _verbose)
                systemPrintf("Calling esp_wifi_set_promiscuous_rx_cb\r\n");
            status = esp_wifi_set_promiscuous_rx_cb(nullptr);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to stop WiFi promiscuous RX callback\r\n");
                break;
            }
            if (_debug)
                systemPrintf("WiFi: Stopped WiFi promiscuous RX callback\r\n");
            _started = _started & ~WIFI_EN_PROMISCUOUS_RX_CALLBACK;
        }

        // Stop promiscuous mode
        if (stopping & WIFI_EN_SET_PROMISCUOUS_MODE)
        {
            if (_debug && _verbose)
                systemPrintf("Calling esp_wifi_set_promiscuous\r\n");
            status = esp_wifi_set_promiscuous(false);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to stop WiFi promiscuous mode, status: %d\r\n", status);
                break;
            }
            if (_debug && _verbose)
                systemPrintf("WiFi: Promiscuous mode stopped\r\n");
            _started = _started & ~WIFI_EN_SET_PROMISCUOUS_MODE;
        }

        // Handle WiFi set channel
        if (stopping & WIFI_EN_SET_CHANNEL)
            _started = _started & ~WIFI_EN_SET_CHANNEL;

        // Stop the long range radio protocols
        if (stopping & WIFI_EN_SET_PROTOCOLS)
        {
            if (!setWiFiProtocols(WIFI_IF_STA, true, false))
                break;
            _started = _started & ~WIFI_EN_SET_PROTOCOLS;
        }

        //****************************************
        // Stop the WiFi station components
        //****************************************

        // Mark the WiFi station offline
        if (stopping & WIFI_STA_ONLINE)
        {
            if (_display && (_started & WIFI_STA_ONLINE))
                systemPrintf("WiFi: Station offline!\r\n");
            _started = _started & ~WIFI_STA_ONLINE;
        }

        // Stop mDNS on WiFi station
        if (stopping & WIFI_STA_START_MDNS)
        {
            if (_started & WIFI_STA_START_MDNS)
            {
                _started = _started & ~WIFI_STA_START_MDNS;
                if (_debug && _verbose)
                    systemPrintf("Calling mDNSStop for WiFi station\r\n");
                mDNSStop();
            }
        }

        // Disconnect from the remote AP
        if (stopping & WIFI_STA_CONNECT_TO_REMOTE_AP)
        {
            if (!stationDisconnect())
                break;
            _started = _started & ~WIFI_STA_CONNECT_TO_REMOTE_AP;
        }

        // Handle auto reconnect
        if (stopping & WIFI_STA_DISABLE_AUTO_RECONNECT)
            _started = _started & ~WIFI_STA_DISABLE_AUTO_RECONNECT;

        // Handle WiFi station host name
        if (stopping & WIFI_STA_SET_HOST_NAME)
            _started = _started & ~WIFI_STA_SET_HOST_NAME;

        // Handle WiFi select channel
        if (stopping & WIFI_SELECT_CHANNEL)
            _started = _started & ~(stopping & WIFI_SELECT_CHANNEL);

        // Handle WiFi station select remote AP
        if (stopping & WIFI_STA_SELECT_REMOTE_AP)
            _started = _started & ~WIFI_STA_SELECT_REMOTE_AP;

        // Handle WiFi station scan
        if (stopping & WIFI_STA_START_SCAN)
            _started = _started & ~WIFI_STA_START_SCAN;

        // Stop the WiFi station radio protocols
        if (stopping & WIFI_STA_SET_PROTOCOLS)
            _started = _started & ~WIFI_STA_SET_PROTOCOLS;

        // Stop station mode
        if (stopping & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE))
        {
            // Determine which bits to clear
            mask = ~(stopping & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE));

            // Stop WiFi station if users are gone
            if (!(_started & mask & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE)))
            {
                if (!setWiFiMode(WIFI_MODE_STA, WIFI_MODE_STA))
                    break;
            }

            // Remove this WiFi station user
            _started = _started & mask;
        }

        //****************************************
        // Stop the soft AP components
        //****************************************

        // Stop soft AP mode
        // Mark the soft AP offline
        if (stopping & WIFI_AP_ONLINE)
        {
            if (_display && softApOnline())
                systemPrintf("WiFi: Soft AP offline!\r\n");
            _started = _started & ~WIFI_AP_ONLINE;
        }

        // Stop mDNS
        if (stopping & WIFI_AP_START_MDNS)
        {
            _started = _started & ~WIFI_AP_START_MDNS;
            if (_debug && _verbose)
                systemPrintf("Calling mDNSStop for soft AP\r\n");
            mDNSStop();
        }

        // Handle the soft AP host name
        if (stopping & WIFI_AP_SET_HOST_NAME)
            _started = _started & ~WIFI_AP_SET_HOST_NAME;

        // Stop soft AP mode
        if (stopping & WIFI_AP_SET_MODE)
        {
            if (!setWiFiMode(WIFI_MODE_AP, WIFI_MODE_AP))
                break;
            _started = _started & ~WIFI_AP_SET_MODE;
        }

        // Disable the radio protocols for soft AP
        if (stopping & WIFI_AP_SET_PROTOCOLS)
            _started = _started & ~WIFI_AP_SET_PROTOCOLS;

        // Stop using the soft AP IP address
        if (stopping & WIFI_AP_SET_IP_ADDR)
            _started = _started & ~WIFI_AP_SET_IP_ADDR;

        // Stop use of SSID and password
        if (stopping & WIFI_AP_SET_SSID_PASSWORD)
            _started = _started & ~WIFI_AP_SET_SSID_PASSWORD;

        //****************************************
        // Stop the common components
        //****************************************

        // Stop the event handler
        if (stopping & WIFI_START_EVENT_HANDLER)
        {
            wifi.eventHandlerStop();
            _started = _started & ~WIFI_START_EVENT_HANDLER;
        }

        // Reset the channel if all components are stopped
        if ((softApOnline() == false) && (stationOnline() == false))
            _channel = 0;

        //****************************************
        // Delay to allow mDNS to shutdown and restart properly
        //****************************************

        delay(100);

        //****************************************
        // Display the items already started and being started
        //****************************************

        if (_debug && _verbose && _started)
            displayComponents("Started", _started);

        if (_debug && _verbose && startingNow)
            displayComponents("Starting", startingNow);

        //****************************************
        // Start the common components
        //****************************************

        // Start the WiFi event handler
        if (starting & WIFI_START_EVENT_HANDLER)
        {
            if (!(_started & WIFI_START_EVENT_HANDLER))
                eventHandlerStart();
            _started = _started | WIFI_START_EVENT_HANDLER;
        }

        //****************************************
        // Start the radio operations
        //****************************************

        // Start the soft AP mode
        if (starting & WIFI_AP_SET_MODE)
        {
            if (!setWiFiMode(WIFI_MODE_AP, 0))
                break;
            _started = _started | WIFI_AP_SET_MODE;
        }

        // Start the station mode
        if (starting & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE))
        {
            if (!setWiFiMode(WIFI_MODE_STA, 0))
                break;
            _started = _started | (starting & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE));
        }

        // Start the soft AP protocols
        if (starting & WIFI_AP_SET_PROTOCOLS)
        {
            if (!setWiFiProtocols(WIFI_IF_AP, true, false))
                break;
            _started = _started | WIFI_AP_SET_PROTOCOLS;
        }

        // Start the WiFi station radio protocols
        if (starting & (WIFI_EN_SET_PROTOCOLS | WIFI_STA_SET_PROTOCOLS))
        {
            bool lrEnable = (starting & WIFI_EN_SET_PROTOCOLS) ? true : false;
            if (!setWiFiProtocols(WIFI_IF_STA, true, lrEnable))
                break;
            _started = _started | (starting & (WIFI_EN_SET_PROTOCOLS | WIFI_STA_SET_PROTOCOLS));
        }

        // Start the WiFi scan for remote APs
        if (starting & WIFI_STA_START_SCAN)
        {
            if (_debug && _verbose)
                systemPrintf("channel: %d\r\n", channel);
            _started = _started | WIFI_STA_START_SCAN;

            // Determine if WiFi scan failed, stop WiFi station startup
            if (wifi.stationScanForAPs(channel) < 0)
            {
                starting &= ~WIFI_STA_FAILED_SCAN;
                starting |= ((_started | starting) & WIFI_AP_ONLINE) ? WIFI_AP_START_MDNS : 0;
                stopping &= ~WIFI_AP_START_MDNS;
                notStarted |= WIFI_STA_FAILED_SCAN;
            }
        }

        // Select an AP from the list
        if (starting & WIFI_STA_SELECT_REMOTE_AP)
        {
            channel = stationSelectAP(_apCount, false);
            _started = _started | WIFI_STA_SELECT_REMOTE_AP;
            if (channel == 0)
            {
                if (_channel)
                    systemPrintf("WiFi STA: No compatible remote AP found on channel %d!\r\n", _channel);
                else
                    systemPrintf("WiFi STA: No compatible remote AP found!\r\n");

                // Stop bringing up WiFi station
                starting &= ~WIFI_STA_NO_REMOTE_AP;
                starting |= ((_started | starting) & WIFI_AP_ONLINE) ? WIFI_AP_START_MDNS : 0;
                stopping &= ~WIFI_AP_START_MDNS;
                notStarted |= WIFI_STA_FAILED_SCAN;
            }
        }

        // Finish the channel selection
        if (starting & WIFI_SELECT_CHANNEL)
        {
            _started = _started | starting & WIFI_SELECT_CHANNEL;
            if (channel & (starting & WIFI_STA_START_SCAN))
            {
                if (_debug && _verbose)
                    systemPrintf("Channel: %d, determined by remote AP scan\r\n",
                                 channel);
            }

            // Use the default channel if necessary
            if (!channel)
                channel = WIFI_DEFAULT_CHANNEL;
            _channel = channel;

            // Display the selected channel
            if (_debug)
                systemPrintf("Channel: %d selected\r\n", _channel);
        }

        //****************************************
        // Start the soft AP components
        //****************************************

        // Set the soft AP SSID and password
        if (starting & WIFI_AP_SET_SSID_PASSWORD)
        {
            if (!softApSetSsidPassword(wifiApSsid, wifiApPassword))
                break;
            _started = _started | WIFI_AP_SET_SSID_PASSWORD;
        }

        // Set the soft AP subnet mask, IP, gateway, DNS, and first DHCP addresses
        if (starting & WIFI_AP_SET_IP_ADDR)
        {
            if (!softApSetIpAddress(_apIpAddress.toString().c_str(),
                                    _apSubnetMask.toString().c_str(),
                                    _apGatewayAddress.toString().c_str(),
                                    _apDnsAddress.toString().c_str(),
                                    _apFirstDhcpAddress.toString().c_str()))
            {
                break;
            }
            _started = _started | WIFI_AP_SET_IP_ADDR;
        }

        // Get the soft AP MAC address
        WiFi.AP.macAddress(_apMacAddress);

        // Set the soft AP host name
        if (starting & WIFI_AP_SET_HOST_NAME)
        {
            // Display the host name
            if (_debug && _verbose)
                systemPrintf("_hostName: %p%s%s%s\r\n", _hostName,
                             _hostName ? " (" : "",
                             _hostName ? _hostName : "",
                             _hostName ? ")" : "");

            // Set the host name
            if (_hostName && (!softApSetHostName(_hostName)))
                break;
            _started = _started | WIFI_AP_SET_HOST_NAME;
        }

        // Start mDNS for the AP network
        if (starting & WIFI_AP_START_MDNS)
        {
            if (!mDNSStart(true))
                break;
            _started = _started | WIFI_AP_START_MDNS;
        }

        // Mark the soft AP as online
        if (starting & WIFI_AP_ONLINE)
        {
            _started = _started | WIFI_AP_ONLINE;

            // Display the soft AP status
            if (_display)
            {
                String mdnsName("");
                if (_started & WIFI_AP_START_MDNS)
                    mdnsName = String(", local.") + String(_hostName);
                systemPrintf("WiFi: Soft AP online, SSID: %s (%s%s), Password: %s\r\n",
                             wifiApSsid,
                             _apIpAddress.toString().c_str(),
                             mdnsName,
                             wifiApPassword);
            }
        }

        // Set the host name
        if (starting & WIFI_STA_SET_HOST_NAME)
        {
            // Display the host name
            if (_debug && _verbose)
                systemPrintf("_hostName: %p%s%s%s\r\n", _hostName,
                             _hostName ? " (" : "",
                             _hostName ? _hostName : "",
                             _hostName ? ")" : "");

            // Set the host name
            if (_hostName && (!stationHostName(_hostName)))
                break;
            _started = _started | WIFI_STA_SET_HOST_NAME;
        }

        // Disable auto reconnect
        if (starting & WIFI_STA_DISABLE_AUTO_RECONNECT)
        {
            if (!WiFi.setAutoReconnect(false))
            {
                systemPrintf("ERROR: Failed to disable auto-reconnect!\r\n");
                break;
            }
            _started = _started | WIFI_STA_DISABLE_AUTO_RECONNECT;
            if (_debug)
                systemPrintf("WiFi auto-reconnect disabled\r\n");
        }

        // Connect to the remote AP
        if (starting & WIFI_STA_CONNECT_TO_REMOTE_AP)
        {
            IPAddress ipAddress;
            uint32_t timer;

            if (!stationConnectAP())
                break;
            _started = _started | WIFI_STA_CONNECT_TO_REMOTE_AP;

            // Wait for an IP address
            ipAddress = WiFi.STA.localIP();
            timer = millis();
            while (!(uint32_t)ipAddress)
            {
                if ((millis() - timer) >= WIFI_IP_ADDRESS_TIMEOUT_MSEC)
                    break;
                delay(10);
                ipAddress = WiFi.STA.localIP();
            }
            if ((millis() - timer) >= WIFI_IP_ADDRESS_TIMEOUT_MSEC)
            {
                systemPrintf("ERROR: Failed to get WiFi station IP address!\r\n");
                break;
            }

            // Save the IP address
            _staHasIp = true;
            _staIpAddress = WiFi.STA.localIP();
            _staIpType = (_staIpAddress.type() == IPv4) ? '4' : '6';
        }

        // Start mDNS for the WiFi station
        if (starting & WIFI_STA_START_MDNS)
        {
            if (mDNSStart(false))
                _started = _started | WIFI_STA_START_MDNS;
        }

        // Mark the station online
        if (starting & WIFI_STA_ONLINE)
        {
            _started = _started | WIFI_STA_ONLINE;
            if (_display)
            {
                String mdnsName("");
                if (_hostName && (_started & WIFI_STA_START_MDNS))
                    mdnsName = String(", local.") + String(_hostName);
                systemPrintf("WiFi: Station online (%s: %s%s)\r\n",
                             _staRemoteApSsid, _staIpAddress.toString().c_str(),
                             mdnsName);
            }
        }

        //****************************************
        // Start the ESP-NOW components
        //****************************************

        // Select the ESP-NOW channel
        if (starting & WIFI_EN_SET_CHANNEL)
        {
            if (_debug && _verbose)
                systemPrintf("Calling esp_wifi_get_channel\r\n");
            status = esp_wifi_get_channel(&primaryChannel, &secondaryChannel);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to get the WiFi channels, status: %d\r\n", status);
                break;
            }
            if (_debug && _verbose)
            {
                systemPrintf("primaryChannel: %d\r\n", primaryChannel);
                systemPrintf("secondaryChannel: %d (%s)\r\n", secondaryChannel,
                             (secondaryChannel == WIFI_SECOND_CHAN_NONE) ? "None"
                             : ((secondaryChannel == WIFI_SECOND_CHAN_ABOVE) ? "Above"
                             : "Below"));
            }

            // Set the ESP-NOW channel
            if (primaryChannel != _channel)
            {
                if (_debug && _verbose)
                    systemPrintf("Calling esp_wifi_set_channel\r\n");
                status = esp_wifi_set_channel(primaryChannel, secondaryChannel);
                if (status != ESP_OK)
                {
                    systemPrintf("ERROR: Failed to set WiFi primary channel to %d, status: %d\r\n", primaryChannel, status);
                    break;
                }
                if (_debug)
                    systemPrintf("WiFi: Set channel %d\r\n", primaryChannel);
            }
            _started = _started | WIFI_EN_SET_CHANNEL;
        }

        // Set promiscuous mode
        if (starting & WIFI_EN_SET_PROMISCUOUS_MODE)
        {
            if (_debug && _verbose)
                systemPrintf("Calling esp_wifi_set_promiscuous\r\n");
            status = esp_wifi_set_promiscuous(true);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to set WiFi promiscuous mode, status: %d\r\n", status);
                break;
            }
            if (_debug && _verbose)
                systemPrintf("WiFi: Enabled promiscuous mode\r\n");
            _started = _started | WIFI_EN_SET_PROMISCUOUS_MODE;
        }

        // Set promiscuous receive callback to get RSSI of action frames
        if (starting & WIFI_EN_PROMISCUOUS_RX_CALLBACK)
        {
            if (_debug && _verbose)
                systemPrintf("Calling esp_wifi_set_promiscuous_rx_cb\r\n");
            status = esp_wifi_set_promiscuous_rx_cb(wifiPromiscuousRxHandler);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to set the WiFi promiscuous RX callback, status: %d\r\n", status);
                break;
            }
            if (_debug && _verbose)
                systemPrintf("WiFi: Promiscuous RX callback established\r\n");
            _started = _started | WIFI_EN_PROMISCUOUS_RX_CALLBACK;
        }

        // Start ESP-NOW
        if (starting & WIFI_EN_START_ESP_NOW)
        {
            if (_debug && _verbose)
                systemPrintf("Calling espNowStart\r\n");
            if (!espNowStart())
            {
                systemPrintf("ERROR: Failed to start ESP-NOW\r\n");
                break;
            }
            if (_debug)
                systemPrintf("WiFi: ESP-NOW started\r\n");
            _started = _started | WIFI_EN_START_ESP_NOW;
        }

        // Mark ESP-NOW online
        if (starting & WIFI_EN_ESP_NOW_ONLINE)
        {
            _started = _started | WIFI_EN_ESP_NOW_ONLINE;
            systemPrintf("WiFi: ESP-NOW online (%02x:%02x:%02x:%02x:%02x:%02x, channel: %d)\r\n",
                         _staMacAddress[0], _staMacAddress[1], _staMacAddress[2],
                         _staMacAddress[3], _staMacAddress[4], _staMacAddress[5],
                         _channel);
        }

        // All components started successfully
        enabled = true;
    } while (0);

    //****************************************
    // Display the items that were not stopped
    //****************************************
    if (_debug && _verbose)
    {
        systemPrintf("0x%08x: stopping\r\n", stopping);
        systemPrintf("0x%08x: _started\r\n", _started);
    }

    // Determine which components were not stopped
    stopping &= _started;
    if (_debug && stopping)
        displayComponents("ERROR: Items NOT stopped", stopping);

    //****************************************
    // Display the items that were not started
    //****************************************

    if (_debug && _verbose && _verbose)
    {
        systemPrintf("0x%08x: startingNow\r\n", startingNow);
        systemPrintf("0x%08x: _started\r\n", _started);
    }
    startingNow &= ~_started;
    if (_debug &&  startingNow)
        displayComponents("ERROR: Items NOT started", startingNow);

    //****************************************
    // Display the items that were not started
    //****************************************

    if (_debug && _verbose)
    {
        systemPrintf("0x%08x: startingNow\r\n", startingNow);
        systemPrintf("0x%08x: _started\r\n", _started);
    }

    // Clear the items that were not started
    _started = _started & ~notStarted;

    if (_debug && _verbose && _started)
        displayComponents("Started items", _started);

    // Return the enable status
    if (!enabled)
        systemPrintf("ERROR: RTK_WIFI::enable failed!\r\n");
    return enabled;
}

//*********************************************************************
// Test the WiFi modes
void RTK_WIFI::test(uint32_t testDurationMsec)
{
    uint32_t currentMsec;
    bool disconnectFirst;
    static uint32_t lastScanMsec = - (1000 * 1000);
    int rand;

    // Delay the mode change until after the WiFi scan completes
    currentMsec = millis();
    if (_scanRunning)
        lastScanMsec = currentMsec;

    // Check if it time for a mode change
    else if ((currentMsec - lastScanMsec) < testDurationMsec)
        return;

    // Perform the test
    lastScanMsec = currentMsec;

    // Get a random number
    rand = random() & 0x1f;
    disconnectFirst = (rand & 0x10) ? true : false;

    // Determine the next actions
    switch (rand)
    {
    default:
        lastScanMsec = currentMsec - (1000 * 1000);
        break;

    case 0:
        systemPrintf("--------------------  %d: All Stop  --------------------\r\n", rand);
        wifi.enable(false, false, false);
        break;

    case 1:
        systemPrintf("--------------------  %d: STA Start  -------------------\r\n", rand);
        wifi.enable(false, false, true);
        break;

    case 2:
        systemPrintf("--------------------  %d: STA Disconnect  --------------\r\n", rand);
        wifi.stationDisconnect();
        break;

    case 4:
        systemPrintf("--------------------  %d: Soft AP Start  -------------------\r\n", rand);
        wifi.enable(false, true, false);
        break;

    case 5:
        systemPrintf("--------------------  %d: Soft AP & STA Start  --------------------\r\n", rand);
        wifi.enable(false, true, true);
        break;

    case 6:
        systemPrintf("--------------------  %d: Soft AP Start, STA Disconnect  -------------------\r\n", rand);
        if (disconnectFirst)
            wifi.stationDisconnect();
        enable(false, true, false);
        if (!disconnectFirst)
            wifi.stationDisconnect();
        break;

    case 8:
        systemPrintf("--------------------  %d: ESP-NOW Start  --------------------\r\n", rand);
        wifi.enable(true, false, false);
        break;

    case 9:
        systemPrintf("--------------------  %d: ESP-NOW & STA Start  -------------------\r\n", rand);
        wifi.enable(true, false, true);
        break;

    case 0xa:
        systemPrintf("--------------------  %d: ESP-NOW Start, STA Disconnect  --------------\r\n", rand);
        if (disconnectFirst)
            wifi.stationDisconnect();
        wifi.enable(true, false, false);
        if (!disconnectFirst)
            wifi.stationDisconnect();
        break;

    case 0xc:
        systemPrintf("--------------------  %d: ESP-NOW & Soft AP Start  -------------------\r\n", rand);
        wifi.enable(true, true, false);
        break;

    case 0xd:
        systemPrintf("--------------------  %d: ESP-NOW, Soft AP & STA Start  --------------------\r\n", rand);
        wifi.enable(true, true, true);
        break;

    case 0xe:
        systemPrintf("--------------------  %d: ESP-NOW & Soft AP Start, STA Disconnect  -------------------\r\n", rand);
        if (disconnectFirst)
            wifi.stationDisconnect();
        enable(true, true, false);
        if (!disconnectFirst)
            wifi.stationDisconnect();
        break;
    }
}

//*********************************************************************
// Enable or disable verbose debug output
// Inputs:
//   enable: Set true to enable verbose debug output
// Outputs:
//   Return the previous enable value
bool RTK_WIFI::verbose(bool enable)
{
    bool oldVerbose;

    oldVerbose = _verbose;
    _verbose = enable;
    if (enable)
        debug(enable);
    return oldVerbose;
}

//*********************************************************************
// Verify the WiFi tables
void RTK_WIFI::verifyTables()
{
    // Verify the authorization name table
    if (WIFI_AUTH_MAX != wifiAuthorizationNameEntries)
    {
        systemPrintf("ERROR: Fix wifiAuthorizationName list to match wifi_auth_mode_t in esp_wifi_types.h!\r\n");
        while (1)
        {
        }
    }

    // Verify the Arduino event name table
    if (ARDUINO_EVENT_MAX != arduinoEventNameEntries)
    {
        systemPrintf("ERROR: Fix arduinoEventName list to match arduino_event_id_t in NetworkEvents.h!\r\n");
        while (1)
        {
        }
    }

    // Verify the start name table
    if (WIFI_MAX_START != (1 << wifiStartNamesEntries))
    {
        systemPrintf("ERROR: Fix wifiStartNames list to match list of defines!\r\n");
        while (1)
        {
        }
    }
}
