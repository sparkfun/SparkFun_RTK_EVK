#include <Network.h>
#include <NetworkClientSecure.h>
#include <PPP.h>

#include "Cert_AWS.h"
#include "Cert_Google.h"

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

//----------------------------------------

#define ARDUINO_EVENT_LARA_ON   ((arduino_event_id_t)-1)

//----------------------------------------

typedef bool (* IS_CONNECTED)();

//----------------------------------------

typedef struct _HTTPS_CLIENT_CONNECTION
{
    IS_CONNECTED isInternetAvailable;
    const char * rootCA;
    const char * hostName;
    const char * url;

    // The following parameters are initialized to zero (false)
    bool suppressFirstPageOutput;
    NetworkClientSecure client;
    int headerLength;
    int pageLength;
    uint8_t hccState;
    bool tagEndFound;
    int tagEndOffset;
    bool tagStartFound;
    int tagStartOffset;
    uint32_t timer;
    uint8_t buffer[2048];
} HTTPS_CLIENT_CONNECTION;

HTTPS_CLIENT_CONNECTION google;
HTTPS_CLIENT_CONNECTION SparkFun;

//----------------------------------------
// System initialization
void setup()
{
    delay(1000);

    Serial.begin(115200);

    Serial.println("SparkFun EVK Example");

    // Initialize the network
    Network.onEvent(networkOnEvent);

    // Set the HTTPS parameters
    google.rootCA = caGoogle;
    google.hostName = "www.google.com";
    google.url = "/";
    google.isInternetAvailable = pppIsInternetAvailable;
    google.suppressFirstPageOutput = true;

    SparkFun.rootCA = caAmazonWebServices;
    SparkFun.hostName = "www.sparkfun.com";
    SparkFun.url = "/";
    SparkFun.isInternetAvailable = pppIsInternetAvailable;
    SparkFun.suppressFirstPageOutput = true;

    // Start LARA
    pppEvent(ARDUINO_EVENT_LARA_ON);
}

//----------------------------------------
// Main loop
void loop()
{
    pppUpdate();
    httpsUpdate(&google, 443, 15 * 1000);
    httpsUpdate(&SparkFun, 443, 16 * 1000);
}
