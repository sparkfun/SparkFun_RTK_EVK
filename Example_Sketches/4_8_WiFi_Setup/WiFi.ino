/**********************************************************************
  Wifi.ino

  Handle the WiFi events
**********************************************************************/

#define DEBUG_WIFI              0
#define DISPLAY_WIFI_MSGS       1

//****************************************
// Constants
//****************************************

#define WIFI_RECONNECTION_DELAY     500 // milliseconds

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
// Handle the WiFi event
void eventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    WIFI_CHANNEL_t channel;
    static IPAddress localIP;
    static char localIpType;
    WIFI_START_LIST_t mask;
    wifi_mode_t mode;
    bool success;

#if DEBUG_WIFI
    systemPrintf("event: %d\r\n", event);
#endif  // DEBUG_WIFI

    // Notify the upper layers that WiFi is no longer available
    if ((event >= ARDUINO_EVENT_WIFI_SCAN_DONE)
        && (event <= ARDUINO_EVENT_WIFI_STA_LOST_IP))
    {
        switch (event)
        {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            break;

        case ARDUINO_EVENT_WIFI_SCAN_DONE:
        case ARDUINO_EVENT_WIFI_STA_START:
        case ARDUINO_EVENT_WIFI_STA_STOP:
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            wifiChannel = 0;
            stationConnected = false;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            stationHasIp = false;
            break;
        }
    }

    // Handle the event
    switch (event)
    {

    //------------------------------
    // Controller events
    //------------------------------

    case ARDUINO_EVENT_WIFI_OFF:
        if (debug)
            systemPrintf("WiFi Event: Off\r\n");
        break;

    case ARDUINO_EVENT_WIFI_READY:
        if (debug)
            systemPrintf("WiFi Event: Ready\r\n");
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

    case ARDUINO_EVENT_WIFI_STA_START:
        if (debug)
            systemPrintf("WiFi Event: Station start\r\n");
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        stationConnected = true;
        if (debug)
            systemPrintf("WiFi Event: Station connected\r\n");
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        stationHasIp = true;

        // Display the IP address
        if (display)
        {
            localIP = WiFi.STA.localIP();
            localIpType = (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) ? '4' : '6';
            systemPrintf("WiFi Event: Got IPv%c address %s\r\n",
                         localIpType, localIP.toString().c_str());
        }

        // Start mDNS
        mDNSStart();
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        if (display)
            systemPrintf("WiFi Event: Lost IPv%c address %s\r\n",
                         localIpType, localIP.toString().c_str());
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    // Argument structure for WIFI_EVENT_STA_DISCONNECTED event
    //  typedef struct {
    //      uint8_t ssid[32];         /**< SSID of disconnected AP */
    //      uint8_t ssid_len;         /**< SSID length of disconnected AP */
    //      uint8_t bssid[6];         /**< BSSID of disconnected AP */
    //      uint8_t reason;           /**< reason of disconnection */
    //      int8_t  rssi;             /**< rssi of disconnection */
    //  } wifi_event_sta_disconnected_t;
        if (display)
            systemPrintf("WiFi Event: Station disconnected from %s\r\n",
                         info.wifi_sta_disconnected.ssid);

        // Start the reconnection timer
        wifiTimer = millis();
        break;

    case ARDUINO_EVENT_WIFI_STA_STOP:
        if (debug)
            systemPrintf("WiFi Event: Station stop\r\n");

        // Stop the reconnection timer
        wifiTimer = 0;
        break;

    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        if (debug)
            systemPrintf("WiFi Event: Station authorization change\r\n");
        break;

    //----------------------------------------
    // Scan events
    //----------------------------------------

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        // Argument structure for WIFI_EVENT_SCAN_DONE event
        //  typedef struct
        //  {
        //      uint32_t status;    // status of scanning APs: 0 — success, 1 - failure
        //      uint8_t  number;    // number of scan results
        //      uint8_t  scan_id;   // scan sequence number, used for block scan
        //  } wifi_event_sta_scan_done_t;
        wifiScanRunning = false;
        do {
            if (info.wifi_scan_done.status != 0)
            {
                if (debug)
                    systemPrintf("WiFi Event: Scan done - failed!\r\n");
                break;
            }
            if (debug)
                systemPrintf("WiFi Event: Scan done\r\n");

            // Select an AP from the list
            uint8_t apCount = info.wifi_scan_done.number;
            channel = stationSelectAP(apCount, false);
            if (channel == 0)
            {
                systemPrintf("No compatible remote AP found!\r\n");
                break;
            }
            wifiChannel = channel;

            // Start WiFi station mode
            mode = WiFi.getMode();
            if (WiFi.mode((wifi_mode_t)(mode | WIFI_MODE_STA)) == false)
            {
                systemPrintf("ERROR: Failed to enable WiFi station mode!\r\n");
                break;
            }
            if (debug)
                systemPrintf("WiFi station mode enabled\r\n");

            // Set the host name
            if (!stationHostName(hostName))
                break;

            // Disable auto reconnect
            if (!WiFi.setAutoReconnect(false))
            {
                systemPrintf("ERROR: Failed to disable auto-reconnect!\r\n");
                break;
            }
            if (debug)
                systemPrintf("WiFi auto-reconnect disabled\r\n");

            // Connect to the remote AP
            stationConnectAP();
        } while (0);
        break;
    }
}

//*********************************************************************
// Start the WiFi event handler
void eventHandlerStart()
{
    if (debug)
        systemPrintf("Starting the WiFi event handler\r\n");

    // Establish the WiFi event handler
    Network.onEvent(eventHandler);
}

//*********************************************************************
// Stop the WiFi event handler
void eventHandlerStop()
{
    if (debug)
        systemPrintf("Stopping the WiFi event handler\r\n");

    Network.removeEvent(eventHandler);
}

//*********************************************************************
// Start the multicast domain name server
// Outputs:
//   Returns true if successful and false upon failure
bool mDNSStart()
{
    bool debug;
    bool started;

    do
    {
        // Verify that a host name exists
        started = (hostName != nullptr) && (strlen(hostName) != 0);
        if (!started)
        {
            systemPrintf("ERROR: No mDNS host name specified!\r\n");
            break;
        }

        // Start mDNS
        if (debug)
            systemPrintf("Starting mDNS\r\n");
        started = MDNS.begin(hostName);
        if (!started)
        {
            systemPrintf("ERROR: Failed to start mDNS!\r\n");
            break;
        }
        if (display || debug)
            systemPrintf("mDNS started as %s.local\r\n", hostName);
    } while (0);
    return started;
}

//*********************************************************************
// Stop the multicast domain name server
void mDNSStop()
{
    if (debug)
        systemPrintf("Stopping mDNS\r\n");
    MDNS.end();
    if (debug)
        systemPrintf("mDNS stopped\r\n");
}

//*********************************************************************
// Connect the station to a remote AP
bool stationConnectAP()
{
    bool connected;

    do
    {
        // Determine if a remote AP is available
        connected = apFound;
        if (!connected)
        {
            systemPrintf("ERROR: No remote AP found!\r\n");
            break;
        }

        // Connect to the remote AP
        if (debug)
            systemPrintf("WiFi connecting to %s on channel %d with %s authorization\r\n",
                         apSSID,
                         wifiChannel,
                         (authType < WIFI_AUTH_MAX) ? wifiAuthorizationName[authType] : "Unknown");
        connected = (WiFi.STA.connect(apSSID, apPassword, wifiChannel));
        if (!connected)
        {
            if (debug)
                systemPrintf("WIFI failed to connect to SSID %s with password %s\r\n", apSSID, apPassword);
            break;
        }
        if (display)
            systemPrintf("WiFi station connected to %s on channel %d with %s authorization\r\n",
                         apSSID,
                         wifiChannel,
                         (authType < WIFI_AUTH_MAX) ? wifiAuthorizationName[authType] : "Unknown");
    } while (0);
    return connected;
}

//*********************************************************************
// Disconnect the station from an AP
// Outputs:
//   Returns true if successful and false upon failure
bool stationDisconnect()
{
    bool disconnected;

    do
    {
        // Determine if station is connected to a remote AP
        disconnected = !stationConnected;
        if (disconnected)
        {
            if (debug)
                systemPrintf("Station already disconnected from remote AP\r\n");
            break;
        }

        // Disconnect from the remote AP
        if (debug)
            systemPrintf("WiFI disconnection station\r\n");
        disconnected = WiFi.STA.disconnect();
        if (!disconnected)
        {
            systemPrintf("ERROR: Failed to disconnect WiFi from the AP!\r\n");
            break;
        }
        if (debug)
            systemPrintf("WiFi disconnected from the AP\r\n");
    } while (0);
    return disconnected;
}

//*********************************************************************
// Set the station's host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool stationHostName(const char * hostName)
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
        if (debug)
            systemPrintf("WiFI setting station host name\r\n");
        nameSet = WiFi.STA.setHostname(hostName);
        if (!nameSet)
        {
            systemPrintf("ERROR: Failed to set the Wifi station host name!\r\n");
            break;
        }
        if (debug)
            systemPrintf("WiFi station hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Start the WiFi scan
// Inputs:
//   channel: Channel number for the scan, zero (0) scan all channels
// Outputs:
//   Returns true if successful and false upon failure
bool stationScanStart(WIFI_CHANNEL_t channel)
{
    int16_t status;

    do
    {
        // Determine if the WiFi scan is already running
        if (wifiScanRunning)
        {
            if (debug)
                systemPrintf("WiFi scan already running");
            break;
        }

        // Determine if scanning a single channel or all channels
        if (debug)
        {
            if (channel)
                systemPrintf("WiFI starting AP scan on channel %d\r\n", channel);
            else
                systemPrintf("WiFI starting AP scan\r\n");
        }

        // Start the WiFi scan
        status = WiFi.scanNetworks(true,        // async
                                   false,       // show_hidden
                                   false,       // passive
                                   300,         // max_ms_per_chan
                                   channel,     // channel number
                                   nullptr,     // ssid *
                                   nullptr);    // bssid *
        wifiScanRunning = (status == WIFI_SCAN_RUNNING);
#if DEBUG_WIFI
        Serial.printf("status: %d\r\n", status);
        Serial.printf("wifiScanRunning: %d\r\n", wifiScanRunning);
#endif  // DEBUG_WIFI
        if (!wifiScanRunning)
        {
            systemPrintf("ERROR: WiFi scan failed to start!\r\n");
            break;
        }
        if (debug)
        {
            if (channel)
                systemPrintf("WiFi scan started on channel %d\r\n", channel);
            else
                systemPrintf("WiFi scan started\r\n");
        }
    } while (0);
    return wifiScanRunning;
}

//*********************************************************************
// Select the AP and channel to use for WiFi station
// Inputs:
//   apCount: Number to APs detected by the WiFi scan
//   list: Determine if the APs should be listed
// Outputs:
//   Returns the channel number of the AP
WIFI_CHANNEL_t stationSelectAP(uint8_t apCount, bool list)
{
    int ap;
    WIFI_CHANNEL_t apChannel;
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
    if (list || debug)
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
                    if (debug)
                        systemPrintf("WiFi: Found remote AP: %s\r\n", ssid);

                    // A match was found, save it and stop looking
                    apSSID = wifiSsidPassword[authIndex].ssid;
                    apPassword = wifiSsidPassword[authIndex].password;
                    apChannel = channel;
                    authType = type;
                    apFound = true;
                    break;
                }
            }

            // Check for done
            if (apFound && (!(debug | list)))
                break;
        }

        // Display the list of APs
        if (list || debug)
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
// Start the WiFi station
// Outputs:
//   Returns true if successful and false upon failure
bool stationStart()
{
    int authIndex;
    wifi_mode_t mode;
    bool started;

    do
    {
        // Verify that at least one WiFi access point is in the list
        started = (wifiSsidPasswordEntries != 0);
        if (!started)
        {
            systemPrintf("ERROR: No entries in wiFiSsidPassword\r\n");
            break;
        }

        // Verify that at least one SSID is set
        for (authIndex = 0; authIndex < wifiSsidPasswordEntries; authIndex++)
            if (wifiSsidPassword[authIndex].ssid
                && (strlen(wifiSsidPassword[authIndex].ssid)))
            {
                break;
            }
        if (authIndex >= wifiSsidPasswordEntries)
        {
            systemPrintf("ERROR: No valid SSID in wifiSsidPassword\r\n");
            break;
        }

        // Determine if WiFi is running
        mode = WiFi.getMode();
#if DEBUG_WIFI
        systemPrintf("mode: %d\r\n", mode);
#endif  // DEBUG_WIFI
        started = ((mode & WIFI_MODE_STA) != 0);
        if (started)
        {
            if (debug)
                systemPrintf("WiFi: Station already started\r\n");
            if (stationConnected)
                break;
        }
        else
        {
            // Start the WiFi event handler
            eventHandlerStart();

            // Attempt to enable WiFi station
            if (debug)
                systemPrintf("WiFi: Starting station mode\r\n");
            started = WiFi.mode((wifi_mode_t)(mode | WIFI_MODE_STA));
            if (!started)
            {
                systemPrintf("ERROR: Failed to stop WiFi station mode!\r\n");
                break;
            }
            if (debug)
                systemPrintf("WiFi: Started station mode\r\n");
        }

        // Start the WiFi scan
        started = stationScanStart(wifiChannel);
    } while (0);
    return started;
}

//*********************************************************************
// Stop the WiFi station
// Outputs:
//   Returns true if successful and false upon failure
bool stationStop()
{
    wifi_mode_t mode;
    bool stopped;

    do {
        // Determine if WiFi station is already stopped
        if (debug)
            systemPrintf("WiFI stopping station mode\r\n");
        mode = WiFi.getMode();
#if DEBUG_WIFI
        systemPrintf("mode: %d\r\n", mode);
#endif  // DEBUG_WIFI
        stopped = ((mode & WIFI_MODE_STA) == 0);
        if (stopped)
        {
            if (debug)
                systemPrintf("WiFi STA already stopped\r\n");
            break;
        }

        // Stop mDNS
        mDNSStop();

        // Disconnect from the remote AP
        stopped = stationDisconnect();
        if (!stopped)
            break;

        // Stop WiFi station
        stopped = WiFi.mode((wifi_mode_t)(mode & ~WIFI_MODE_STA));
        if (!stopped)
        {
            systemPrintf("ERROR: Failed to stop WiFi STA!\r\n");
            break;
        }
        if (debug)
            systemPrintf("WiFI STA: Started --> Stopped\r\n");

        // Remove the WiFi event handler
        eventHandlerStop();
    } while (0);
    return stopped;
}

//*********************************************************************
// Test the WiFi modes
void update()
{
    uint32_t currentMsec;
    static uint32_t lastScanMsec = - (180 * 1000);
    int rand;

    // Check for reconnection request
    if (wifiTimer)
    {
        if ((millis() - wifiTimer) >= WIFI_RECONNECTION_DELAY)
        {
            wifiTimer = 0;

            // Start the WiFi scan
            stationScanStart(wifiChannel);
        }
    }

    // Delay the mode change until after the WiFi scan completes
    currentMsec = millis();
    if (wifiScanRunning)
        lastScanMsec = currentMsec;

    // Check if it time for a mode change
    else if ((currentMsec - lastScanMsec) >= (5 * 1000))
    {
        lastScanMsec = currentMsec;

        // Get a random number
        rand = random() & 3;

        // Determine the next actions
        switch (rand)
        {
        default:
            lastScanMsec = 0;
            break;

        case 0:
            systemPrintf("--------------------  %d: Stop  --------------------\r\n", rand);
            stationStop();
            break;

        case 1:
            systemPrintf("--------------------  %d: Start  -------------------\r\n", rand);
            stationStart();
            break;

        case 2:
            systemPrintf("--------------------  %d: Disconnect  --------------\r\n", rand);
            stationDisconnect();
            break;
        }
    }
}

//*********************************************************************
// Setup WiFi
void wifiBegin()
{
    // Enable / disable debug
#if DEBUG_WIFI
    debug = true;
#else   // DISPLAY_DEBUG_MSGS
    debug = false;
#endif  // DISPLAY_DEBUG_MSGS

    // Enable / disable message display
#if DISPLAY_WIFI_MSGS
    display = true;
#else   // DISPLAY_WIFI_MSGS
    display = false;
#endif  // DISPLAY_WIFI_MSGS
    if (debug)
        display = true;
}
