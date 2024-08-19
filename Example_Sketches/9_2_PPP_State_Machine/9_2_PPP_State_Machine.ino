#include <PPP.h>

//----------------------------------------

#define ARDUINO_EVENT_LARA_ON   ((arduino_event_id_t)-1)

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

//----------------------------------------
// System initialization
void setup()
{
    Serial.begin(115200);

    // Initialize the network
    Network.onEvent(networkOnEvent);

    // Set the HTTPS parameters
    google.hostName = "www.google.com";
    google.url = "/";
    google.isInternetAvailable = pppIsInternetAvailable;
//    google.suppressFirstPageOutput = true;

    SparkFun.hostName = "www.SparkFun.com";
    SparkFun.url = "/";
    SparkFun.isInternetAvailable = pppIsInternetAvailable;
//    SparkFun.suppressFirstPageOutput = true;

    // Start LARA
    pppEvent(ARDUINO_EVENT_LARA_ON);
}

//----------------------------------------
// Main loop
void loop()
{
    pppUpdate();
    httpUpdate(&google, 80, 15 * 1000);
    httpUpdate(&SparkFun, 80, 16 * 1000);
}
