/**********************************************************************
  Wifi.ino

  Handle the WiFi events
**********************************************************************/

#define DEBUG_WIFI              1
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
  ARDUINO_EVENT_WIFI_AP_START:
  ARDUINO_EVENT_WIFI_AP_STOP:
  ARDUINO_EVENT_WIFI_AP_STACONNECTED:
  ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
  ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
  ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
  ARDUINO_EVENT_WIFI_AP_GOT_IP6:
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
    // WiFi AP State Machine
    //
    //    .----------+<----------+<------------+<---------------+<-----------------+<-------------------.
    //    v          |           |             |                |                  |                    |
    // AP_STOP --> READY --> AP_GOT_IP6 --> AP_START --> AP_STACONNECTED --> STAIPASSIGNED --> AP_STADISCONNECTED
    //               ^                                          ^                  |                    |
    //               |                                          |                  v                    |
    // OFF ----------'                                          '------------------+<-------------------'
    //
    // Handle the WiFi station events
    //------------------------------

    case ARDUINO_EVENT_WIFI_AP_STOP:
        if (debug)
            systemPrintf("WiFi Event: AP Stop\r\n");

        // Indicate that the soft AP is no longer available
        softApRunning = false;
        softApHasIp = false;
        break;

    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        if (debug)
            systemPrintf("WiFi Event: AP Got IPv6 address\r\n");

        // Indicate that the soft AP has an IP address
        softApHasIp = true;
        break;

    case ARDUINO_EVENT_WIFI_AP_START:
        if (debug)
            systemPrintf("WiFi Event: AP Start\r\n");

        // Indicate that the soft AP is available
        softApRunning = true;
        softApHasIp = true;
        break;

    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        // Account for this station
        softApStationCount += 1;
        if (debug)
            systemPrintf("WiFi Event: Station connected to soft AP, count: %d\r\n", softApStationCount);
        break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        // Account for this station
        softApStationCount -= 1;
        if (debug)
            systemPrintf("WiFi Event: Station disconnected from soft AP, count; %d\r\n", softApStationCount);
        break;

    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        if (debug)
            systemPrintf("WiFi Event: AP assigned station IP address\r\n");
        break;

    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        if (debug)
            systemPrintf("WiFi Event: AP probe received\r\n");
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
        // Only start mDNS once
        started = mDnsRunning;
        if (started)
        {
            if (debug)
                systemPrintf("mDNS already running\r\n");
            break;
        }

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
        mDnsRunning = true;
        if (display || debug)
            systemPrintf("mDNS started as %s.local\r\n", hostName);
    } while (0);
    return started;
}

//*********************************************************************
// Stop the multicast domain name server
void mDNSStop()
{
    if (mDnsRunning)
    {
        if (debug)
            systemPrintf("Stopping mDNS\r\n");
        MDNS.end();
        mDnsRunning = false;
        if (debug)
            systemPrintf("mDNS stopped\r\n");
    }
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
bool softApConfiguration(const char * ipAddress,
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
            uDhcpFirstAddress = IPAddress("192.168.4.2");
        else
            uDhcpFirstAddress = 0;
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
    if (display)
    {
        systemPrintf("Soft AP configuration:\r\n");
        systemPrintf("    %s: IP Address\r\n", IPAddress(uIpAddress).toString().c_str());
        systemPrintf("    %s: Subnet mask\r\n", IPAddress(uSubnetMask).toString().c_str());
        if (uGatewayAddress)
            systemPrintf("    %s: Gateway address\r\n", IPAddress(uGatewayAddress).toString().c_str());
        systemPrintf("    %s: First DHCP address\r\n", IPAddress(uDhcpFirstAddress).toString().c_str());
        systemPrintf("    %s: DNS address\r\n", IPAddress(uDnsAddress).toString().c_str());
    }

    // Configure the soft AP
    configured = WiFi.AP.config(uIpAddress,
                                uGatewayAddress,
                                uSubnetMask,
                                uDhcpFirstAddress,
                                uDnsAddress,
                                IPAddress((uint32_t)0));
    if (!configured)
        systemPrintf("ERROR: Failed to configure the soft AP with IP addresses!\r\n");
    return configured;
}

//*********************************************************************
// Set the soft AP SSID and password
// Outputs:
//   Returns true if successful and false upon failure
bool softApCreate(const char * ssid, const char * password)
{
    bool created;

    // Set the WiFi soft AP SSID and password
    if (debug)
        systemPrintf("WiFi AP: Attempting to set AP SSID and password\r\n");
    created = WiFi.AP.create(ssid, password);
    if (!created)
        systemPrintf("ERROR: Failed to set soft AP SSID and Password!\r\n");
    else if (display)
        systemPrintf("WiFi AP: SSID: %s, Password: %s\r\n", ssid, password);
    return created;
}

//*********************************************************************
// Set the soft AP host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool softApHostName(const char * hostName)
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
            systemPrintf("WiFI setting AP host name\r\n");
        nameSet = WiFi.AP.setHostname(hostName);
        if (!nameSet)
        {
            systemPrintf("ERROR: Failed to set the Wifi AP host name!\r\n");
            break;
        }
        if (debug)
            systemPrintf("WiFi AP hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Start the soft AP
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool softApStart()
{
    wifi_mode_t mode;
    uint8_t protocols;
    bool started;
    esp_err_t status;

    do
    {
        // Determine if WiFi is running
        mode = WiFi.getMode();
#if DEBUG_WIFI
        systemPrintf("mode: %d\r\n", mode);
#endif  // DEBUG_WIFI
        started = ((mode & (WIFI_MODE_STA | WIFI_MODE_AP)) != 0);
        if (started)
        {
            if (debug)
                systemPrintf("WiFi AP: Already started\r\n");
            break;
        }

        // Establish the WiFi event handler
        eventHandlerStart();

        // Assign the AP SSID and Password
        started = softApCreate(wifiApSSID, wifiApPassword);
        if (!started)
            break;

        // Set the IP addresses
        started = softApConfiguration(apIpAddress,
                                      apSubnetMask,
                                      apGatewayAddress,
                                      apDnsAddress,
                                      apDhcpFirstAddress);
        if (!started)
            break;

        // Determine the protocols that are enabled
        protocols = 0;
        status = esp_wifi_get_protocol(WIFI_IF_AP, &protocols);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: esp_wifi_get_protocol failed, status: 0x%x!\r\n", status);
            break;
        }

        // Make sure the 802.3 protocols are enabled
        protocols |= WIFI_PROTOCOL_11B
                   | WIFI_PROTOCOL_11N
//                   | WIFI_PROTOCOL_11AX
                   | WIFI_PROTOCOL_11G;
        status = esp_wifi_set_protocol(WIFI_IF_AP, protocols);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: esp_wifi_set_protocol failed, status: 0x%x!\r\n", status);
            break;
        }

        // Attempt to enable WiFi soft AP
        if (debug)
            systemPrintf("WiFi AP: Starting soft AP\r\n");
        started = WiFi.mode((wifi_mode_t)(mode | WIFI_MODE_AP));
        if (!started)
        {
            systemPrintf("ERROR: Failed to start soft AP mode!\r\n");
            break;
        }
        if (display)
            systemPrintf("WiFi AP: Soft AP started, IP address: %s\r\n",
                         WiFi.AP.localIP().toString().c_str());

        // Set the host address
        started = softApHostName(hostName);
        if (!started)
            break;

        // Start mDNS
        started = mDNSStart();
    } while (0);
    return started;
}

//*********************************************************************
// Stop the soft AP
// Outputs:
//   Returns true if successful and false upon failure
bool softApStop()
{
    bool stopped;
    wifi_mode_t mode;

    do
    {
        // Stop mDNS
        mDNSStop();

        // Stop the soft AP
        mode = WiFi.getMode();
#if DEBUG_WIFI
        systemPrintf("mode: %d\r\n", mode);
#endif  // DEBUG_WIFI
        stopped = WiFi.mode((wifi_mode_t)(mode & ~WIFI_MODE_AP));
        if (!stopped)
        {
            systemPrintf("ERROR: Failed to stop soft AP mode!\r\n");
            break;
        }

        // Remove the WiFi event handler
        eventHandlerStop();
    } while (0);
    return stopped;
}

//*********************************************************************
// Test the WiFi modes
void wifiUpdate()
{
    uint32_t currentMsec;
    static uint32_t lastScanMsec = - (180 * 1000);
    int rand;

    // Check if it time for a mode change
    currentMsec = millis();
    if ((currentMsec - lastScanMsec) >= (30 * 1000))    //(5 * 1000))
    {
        lastScanMsec = currentMsec;

        // Get a random number
        rand = random() & 1;

        // Determine the next actions
        switch (rand)
        {
        default:
            lastScanMsec = 0;
            break;

        case 0:
            systemPrintf("--------------------  %d: Stop  --------------------\r\n", rand);
            serverStop();
            softApStop();
            break;

        case 1:
            systemPrintf("--------------------  %d: Start  -------------------\r\n", rand);
            if (softApStart())
                serverBegin(WiFi.softAPIP(), SERVER_PORT);
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
