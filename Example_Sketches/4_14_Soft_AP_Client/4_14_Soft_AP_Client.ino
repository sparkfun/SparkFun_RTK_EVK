/**********************************************************************
  4_14_Soft_AP_Client.ino

  Example sketch to test soft AP mode with example 4_13_Muliple_WiFi_Users
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <Arduino.h>    // Built-in
#include <Network.h>    // Built-in
#include <WiFi.h>       // Built-in

#include <secrets.h>    // Host name, SSIDs and passwords

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

#define systemPrintf    Serial.printf
#define systemPrintln   Serial.println

//****************************************
// HTTP client connection class
//****************************************

class HTTP_CLIENT_CONNECTION
{
  private:
    const char * _hostName;
    const char * _url;

    NetworkClient * _client;
    int _headerLength;
    int _pageLength;
    uint16_t _portNumber;
    uint8_t _hccState;
    bool _tagEndFound;
    int _tagEndOffset;
    bool _tagStartFound;
    int _tagStartOffset;
    uint32_t _timer;
    uint8_t _buffer[2048];

  public:

    bool _suppressFirstPageOutput;

    // Constructor
    // Inputs:
    //   hostName: Name of the remote host
    //   portNumber: Port number on the remote host for HTTP connection
    //   url: Web page address
    HTTP_CLIENT_CONNECTION(const char * hostName,
                           uint16_t portNumber,
                           const char * url);

    // Destructor
    ~HTTP_CLIENT_CONNECTION();

    // Read the HTTP page
    // Inputs:
    //   networkConnected: True while the network is connected
    void update(bool networkConnected);
};

HTTP_CLIENT_CONNECTION softApServer("192.168.4.1", 80, "/");

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    systemPrintln();
    systemPrintf(__FILE__ "\r\n");


    // Set the HTTP parameters
    softApServer._suppressFirstPageOutput = true;
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    static uint32_t lastConnectMsec = -1000 * 1000;
    wl_status_t status;
    bool wifiConnected;

    // Determine if WiFi is online
    status = WiFi.status();
    if (status == WL_STOPPED)
    {
        if ((millis() - lastConnectMsec) >= (1 * 1000))
        {
            lastConnectMsec = millis();
            httpUpdate(&softApServer, false);
            status = WiFi.begin(wifiApSsid, wifiApPassword);
        }
    }
    else if ((status == WL_NO_SSID_AVAIL) || (status == WL_CONNECT_FAILED))
    {
        WiFi.STA.end();
        lastConnectMsec = millis();
    }

    wifiConnected = (status == WL_CONNECTED);
    if (wifiConnected)
    {
        if ((millis() - lastConnectMsec) < 2 * 1000)
            httpUpdate(&softApServer, wifiConnected);
        else
        {
            // Retry the HTTP connection
            lastConnectMsec = millis();
            httpUpdate(&softApServer, false);
            WiFi.STA.end();
        }
    }
    else
        httpUpdate(&softApServer, wifiConnected);
}
