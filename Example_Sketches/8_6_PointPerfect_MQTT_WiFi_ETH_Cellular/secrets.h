static const char WIFI_SSID[] = "<ADD YOUR WIFI SSID HERE>";
static const char WIFI_PASSWORD[] = "<ADD YOUR WIFI PASSWORD HERE>";

static const char POINTPERFECT_TOKEN[] = "<ADD YOUR POINTPERFECT TOKEN HERE>"; // "ABCDABCD-ABCD-ABCD-ABCD-ABCDABCDABCD"
static const char ZTP_HARDWARE_ID[] = "<ADD YOUR 14 CHARACTER HARDWARE ID HERE>";  // "ABCDABCDABCD00"
static const char ZTP_GIVEN_NAME[] = "<ADD YOUR FULL THING NAME HERE>"; // "EVK d99.99 - ABCDABCDABCD00"

// Settings for ZTP request that use HTTPS protocol
#define    THINGSTREAM_SERVER     "api.thingstream.io"            //!< the thingstream Rest API server domain        
#define    THINGSTREAM_ZTPPATH    "/ztp/pointperfect/credentials" //!< ZTP rest api
const char THINGSTREAM_ZTPURL[]   = "https://" THINGSTREAM_SERVER THINGSTREAM_ZTPPATH; // full ZTP url

// <Your PointPerfect Thing> -> Credentials -> Hostname
const char AWS_IOT_ENDPOINT[] = "pp.services.u-blox.com";
const unsigned short AWS_IOT_PORT = 8883;

// <Your PointPerfect Thing> -> Credentials -> IP key distribution topic
const char MQTT_TOPIC_KEY[] = "/pp/ubx/0236/Lb"; // This topic provides the L-Band and L-Band + IP dynamic keys in UBX format
// const char MQTT_TOPIC_KEY[] = "/pp/ubx/0236/ip"; // This topic provides the IP only dynamic keys in UBX format

// <Your PointPerfect Thing> -> Credentials -> IP correction topic for EU/US region
const char MQTT_TOPIC_SPARTN[] = "/pp/Lb/us"; // This topic provides the SPARTN corrections for L-Band and L-Band + IP for the US region
// const char MQTT_TOPIC_SPARTN[] = "/pp/Lb/eu"; // This topic provides the SPARTN corrections for L-Band and L-Band + IP for the EU region
// const char MQTT_TOPIC_SPARTN[] = "/pp/ip/us"; // This topic provides the SPARTN corrections for IP only for the US region
// const char MQTT_TOPIC_SPARTN[] = "/pp/ip/eu"; // This topic provides the SPARTN corrections for IP only for the EU region

const bool useLocalizedDistribution = true; // True for localized distribution. Set to false to only use the continental corrections
const char localizedPrefix[] = "pp/ip/L"; // The localized distribution topic prefix. Note: starts with "pp", not "/pp"
const char localizedLevel = '2'; // The localized distribution level: 0 = 10 degrees; 1 = 5 degrees; 2 = 2.5 degrees

// <Your PointPerfect Thing> -> Credentials -> AssistNow (MGA) topic
const bool useAssistNow = true; // Set to true to use Assist Now MGA
const char MQTT_TOPIC_ASSISTNOW[] = "/pp/ubx/mga";
const bool useAssistNowUpdates = true; // If useAssistNow is true, set to true to use updates (less traffic)
const char MQTT_TOPIC_ASSISTNOW_UPDATES[] = "/pp/ubx/mga/updates";

// Amazon Root Certificate
static const char AWS_CERT_CA[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----)EOF";

