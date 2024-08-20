#include <Network.h>
#include <NetworkClientSecure.h>
#include <WiFi.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>

#include "esp_netif.h"
#include "secrets.h"

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

//----------------------------------------

typedef bool (* IS_CONNECTED)();

//----------------------------------------

typedef struct _HTTP_CLIENT_CONNECTION
{
    IS_CONNECTED isInternetAvailable;
    const char * hostName;
    const char * url;

    // The following parameters are initialized to zero (false)
    bool suppressFirstPageOutput;
    NetworkClient client;
    int headerLength;
    int pageLength;
    uint8_t hccState;
    bool tagEndFound;
    int tagEndOffset;
    bool tagStartFound;
    int tagStartOffset;
    uint32_t timer;
    uint8_t buffer[2048];
} HTTP_CLIENT_CONNECTION;

HTTP_CLIENT_CONNECTION google;
HTTP_CLIENT_CONNECTION SparkFun;

WiFiMulti wifiMulti;

//----------------------------------------
void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("SparkFun RTK EVK - Test Sketch");

    // Initialize the network
    Network.onEvent(networkOnEvent);

    // Initialize WiFi
    wifiMulti.addAP(wifiSSID, wifiPassword);
    wifiMulti.run();

    // Set the HTTPS parameters
    google.hostName = "www.google.com";
    google.url = "/";
    google.isInternetAvailable = wifiIsInternetAvailable;
//    google.suppressFirstPageOutput = true;

    SparkFun.hostName = "www.SparkFun.com";
    SparkFun.url = "/";
    SparkFun.isInternetAvailable = wifiIsInternetAvailable;
//    SparkFun.suppressFirstPageOutput = true;
}

//----------------------------------------
void loop()
{
    httpUpdate(&google, 80, 15 * 1000);
    httpUpdate(&SparkFun, 80, 16 * 1000);
}
