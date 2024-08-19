#include <PPP.h>
#include "secrets.h"
#include "base64.h" //Built-in. Needed for NTRIP Client credential encoding.

//----------------------------------------

#define ARDUINO_EVENT_LARA_ON   ((arduino_event_id_t)-1)

#define TIME_ZONE_HOURS         -10
#define TIME_ZONE_MINUTES       0
#define TIME_ZONE_SECONDS       0

//----------------------------------------

bool debugCorrections = false;
bool debugNtripClientRtcm = true;
bool debugNtripClientState = true;
const char * platformPrefix = "EVK";

//----------------------------------------

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
SFE_UBLOX_GNSS myGNSS;
bool gnssOnline;
bool gnssClientUnitsFeetInches = true;

float gnssHorizontalAccuracy;
double gnssLatitude;
double gnssLongitude;

float gnssAltitude;

uint8_t gnssDay;
uint8_t gnssMonth;
uint16_t gnssYear;
uint8_t gnssHour;
uint8_t gnssMinute;
uint8_t gnssSecond;
int32_t gnssNanosecond;
uint16_t gnssMillisecond; // Limited to first two digits

uint8_t gnssSatellitesInView;
uint8_t gnssFixType;
uint8_t gnssCarrierSolution;

bool gnssValidDate;
bool gnssValidTime;
bool gnssConfirmedDate;
bool gnssConfirmedTime;
bool gnssFullyResolved;
uint32_t gnssTAcc;

#define MILLIMETERS_PER_INCH    25.4
#define MILLIMETERS_PER_FOOT    (MILLIMETERS_PER_INCH * 12.)

//----------------------------------------

const int SDA_1 = 21; // ZED-F9P and NEO-D9S
const int SCL_1 = 22; // ZED-F9P and NEO-D9S

#include <Wire.h>
TwoWire I2C_1 = TwoWire(0);

//----------------------------------------

#define NTRIP_CLIENT_PUSHING_RTCM       ntripClientIsPushingRtcm()

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

    // Start I2C. Connect to the GNSS.
    I2C_1.begin((int)SDA_1, (int)SCL_1); //Start I2C

    // Start the GNSS device
    gnssSetup(I2C_1);

    // Set the HTTPS parameters
    google.hostName = "www.google.com";
    google.url = "/";
    google.isInternetAvailable = pppIsInternetAvailable;
    google.suppressFirstPageOutput = true;

    SparkFun.hostName = "www.SparkFun.com";
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
    ntripClientUpdate();
    gnssUpdate();
    httpUpdate(&google, 80, 15 * 1000);
    httpUpdate(&SparkFun, 80, 16 * 1000);
    gnssDisplayLocation(5 * 1000);
}
