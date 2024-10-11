/**********************************************************************
  4_13_Multiple_WiFi_Users.ino

  Example sketch to demonstrate shared WiFi usage
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <Arduino.h>    // Built-in
#include <ESPmDNS.h>    // Built-in
#include <MacAddress.h> // Built-in
#include <Network.h>    // Built-in
#include <WiFi.h>       // Built-in

#include <esp_wifi.h>   // IDF built-in

#include <secrets.h>    // Host name, SSIDs and passwords

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

#define systemPrintf    Serial.printf
#define systemPrintln   Serial.println

//****************************************
// WiFi class
//****************************************

// Handle the WiFi event
// Inputs:
//   event: Arduino ESP32 event number found on
//          https://github.com/espressif/arduino-esp32
//          in libraries/Network/src/NetworkEvents.h
//   info: Additional data about the event
void wifiEventHandler(arduino_event_id_t event, arduino_event_info_t info);

typedef uint32_t WIFI_ACTION_t;
typedef uint8_t WIFI_CHANNEL_t;

// Class to simplify WiFi handling
class RTK_WIFI
{
  private:

    WIFI_CHANNEL_t _apChannel;  // Channel required for soft AP, zero (0) use _channel
    IPAddress _apDnsAddress;    // DNS IP address to use while translating names into IP addresses
    IPAddress _apFirstDhcpAddress;  // First IP address to use for DHCP
    IPAddress _apGatewayAddress;// IP address of the gateway to the larger network (internet?)
    IPAddress _apIpAddress;     // IP address of the soft AP
    uint8_t _apMacAddress[6];   // MAC address of the soft AP
    IPAddress _apSubnetMask;    // Subnet mask for soft AP
    WIFI_CHANNEL_t _channel;    // Current WiFi channel number
    bool _debug;                // Set to true to output debugging data
    bool _display;              // Set to true to display state data
    WIFI_CHANNEL_t _espNowChannel;  // Channel required for ESPNow, zero (0) use _channel
    bool _espNowStarted;        // ESPNow started or running
    const char * _hostName;     // Name of this host
    volatile bool _scanRunning; // Scan running
    int _staAuthType;           // Authorization type for the remote AP
    bool _staConnected;         // True when station is connected
    bool _staHasIp;             // True when station has IP address
    IPAddress _staIpAddress;    // IP address of the station
    uint8_t _staIpType;         // 4 or 6 when IP address is assigned
    uint8_t _staMacAddress[6];  // MAC address of the station
    const char * _staRemoteApSsid;      // SSID of remote AP
    const char * _staRemoteApPassword;  // Password of remote AP
    WIFI_ACTION_t _started;         // Components that are started and running
    WIFI_CHANNEL_t _stationChannel; // Channel required for station, zero (0) use _channel
    bool _stationRunning;       // True while station is starting or running
    uint32_t _timer;            // Reconnection timer
    bool _verbose;              // True causes more debug output to be displayed

    // Start the WiFi event handler
    void eventHandlerStart();

    // Stop the WiFi event handler
    void eventHandlerStop();

    // Start the multicast domain name server
    // Inputs:
    //   softAp: True when starting mDNS on the soft AP
    // Outputs:
    //   Returns true if successful and false upon failure
    bool mDNSStart(bool softAP);

    // Stop the multicast domain name server
    void mDNSStop();

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
    bool setWiFiMode(uint8_t setMode, uint8_t xorMode);

    // Set the WiFi radio protocols
    // Inputs:
    //   interface: Interface on which to set the protocols
    //   enableWiFiProtocols: When true, enable the WiFi protocols
    //   enableLongRangeProtocol: When true, enable the long range protocol
    // Outputs:
    //   Returns true if successful and false upon failure
    bool setWiFiProtocols(wifi_interface_t interface,
                                    bool enableWiFiProtocols,
                                    bool enableLongRangeProtocol);

    // Handle the soft AP events
    // Inputs:
    //   event: Arduino ESP32 event number found on
    //          https://github.com/espressif/arduino-esp32
    //          in libraries/Network/src/NetworkEvents.h
    //   info: Additional data about the event
    void softApEventHandler(arduino_event_id_t event, arduino_event_info_t info);

    // Set the soft AP host name
    // Inputs:
    //   hostName: Zero terminated host name character string
    // Outputs:
    //   Returns true if successful and false upon failure
    bool softApSetHostName(const char * hostName);

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
    bool softApSetIpAddress(const char * ipAddress,
                            const char * subnetMask,
                            const char * gatewayAddress,
                            const char * dnsAddress,
                            const char * dhcpFirstAddress);

    // Set the soft AP SSID and password
    // Outputs:
    //   Returns true if successful and false upon failure
    bool softApSetSsidPassword(const char * ssid, const char * password);

    // Connect to an access point
    // Outputs:
    //   Return true if the connection was successful and false upon failure.
    bool stationConnectAP();

    // Disconnect the station from an AP
    // Outputs:
    //   Returns true if successful and false upon failure
    bool stationDisconnect();

    // Handle the WiFi station events
    // Inputs:
    //   event: Arduino ESP32 event number found on
    //          https://github.com/espressif/arduino-esp32
    //          in libraries/Network/src/NetworkEvents.h
    //   info: Additional data about the event
    void stationEventHandler(arduino_event_id_t event, arduino_event_info_t info);

    // Set the station's host name
    // Inputs:
    //   hostName: Zero terminated host name character string
    // Outputs:
    //   Returns true if successful and false upon failure
    bool stationHostName(const char * hostName);

    // Start the WiFi scan
    // Inputs:
    //   channel: Channel number for the scan, zero (0) scan all channels
    // Outputs:
    //   Returns true if successful and false upon failure
    bool stationScanStart(WIFI_CHANNEL_t channel);

    // Select the AP and channel to use for WiFi station
    // Inputs:
    //   apCount: Number to APs detected by the WiFi scan
    //   list: Determine if the APs should be listed
    // Outputs:
    //   Returns the channel number of the AP
    WIFI_CHANNEL_t stationSelectAP(uint8_t apCount, bool list);

    // Handle the WiFi event
    // Inputs:
    //   event: Arduino ESP32 event number found on
    //          https://github.com/espressif/arduino-esp32
    //          in libraries/Network/src/NetworkEvents.h
    //   info: Additional data about the event
    void wifiEvent(arduino_event_id_t event, arduino_event_info_t info);

  public:

    // Constructor
    // Inputs:
    //   display: Set to true to display WiFi startup summary
    //   debug: Set to true to display WiFi debugging messages
    RTK_WIFI(bool display = false, bool debug = false);

    // Enable or disable WiFi debug
    // Inputs:
    //   enable: Set true to enable debug
    // Outputs:
    //   Return the previous enable value
    bool debug(bool enable);

    // Enable or disable WiFi display
    // Inputs:
    //   enable: Set true to enable display
    // Outputs:
    //   Return the previous enable value
    bool display(bool enable);

    // Enable or disable the WiFi modes
    // Inputs:
    //   enableESPNow: Enable ESP-NOW mode
    //   enableSoftAP: Enable soft AP mode
    //   enableStataion: Enable station mode
    // Outputs:
    //   Returns true if the modes were successfully configured
    bool enable(bool enableESPNow, bool enableSoftAP, bool enableStation);

    // Handle the WiFi event
    // Inputs:
    //   event: Arduino ESP32 event number found on
    //          https://github.com/espressif/arduino-esp32
    //          in libraries/Network/src/NetworkEvents.h
    //   info: Additional data about the event
    void eventHandler(arduino_event_id_t event, arduino_event_info_t info);

    // Get the mDNS host name
    // Outputs:
    //   Returns the mDNS host name as a pointer to a zero terminated string
    //   of characters
    const char * hostNameGet();

    // Set the mDNS host name
    // Inputs
    //   Address of a zero terminated string containing the mDNS host name
    void hostNameSet(const char * mDnsHostName);

    // Configure the soft AP
    // Inputs:
    //   ipAddress: IP address of the soft AP
    //   subnetMask: Subnet mask for the soft AP network
    //   firstDhcpAddress: First IP address to use in the DHCP range
    //   dnsAddress: IP address to use for DNS lookup (translate name to IP address)
    //   gatewayAddress: IP address of the gateway to a larger network (internet?)
    // Outputs:
    //   Returns true if the soft AP was successfully configured.
    bool softApConfiguration(IPAddress ipAddress,
                             IPAddress subnetMask,
                             IPAddress firstDhcpAddress,
                             IPAddress dnsAddress,
                             IPAddress gateway);

    // Display the soft AP configuration
    // Inputs:
    //   display: Address of a Print object
    void softApConfigurationDisplay(Print * display);

    // Get the soft AP status
    // Outputs:
    //   Returns true when the soft AP is ready for use
    bool softApOnline();

    // Get the station status
    // Outputs:
    //   Returns true when the WiFi station is online and ready for use
    bool stationOnline();

    // Handle WiFi station reconnection requests
    void stationReconnectionRequest();

    // Get the station status
    // Outputs:
    //  Returns true if the WiFi station is being started or is online
    bool stationRunning();

    // Test the WiFi modes
    void test();

    // Enable or disable verbose debug output
    // Inputs:
    //   enable: Set true to enable verbose debug output
    // Outputs:
    //   Return the previous enable value
    bool verbose(bool enable);

    // Verify the WiFi tables
    void verifyTables();
};

RTK_WIFI wifi(false, false);

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    systemPrintln();
    systemPrintf(__FILE__ "\r\n");

    // Verity the WiFi tables
    wifi.verifyTables();

    // Enable WiFi debugging
//    wifi.verbose(true);
//    wifi.debug(true);
    wifi.display(true);

    // Set the mDNS host name
    wifi.hostNameSet(mdnsHostName);
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    wifi.stationReconnectionRequest();
    wifi.test();
}
