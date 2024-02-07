// Enter your PointPerfect Thing credentials here:

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

const char localizedPrefix[] = "pp/ip/L"; // The localized distribution topic prefix. Note: starts with "pp", not "/pp"
const char localizedLevel = '2'; // The localized distribution level: 0 = 10 degrees; 1 = 5 degrees; 2 = 2.5 degrees

// <Your PointPerfect Thing> -> Credentials -> AssistNow (MGA) topic
const char MQTT_TOPIC_ASSISTNOW[] = "/pp/ubx/mga";
const char MQTT_TOPIC_ASSISTNOW_UPDATES[] = "/pp/ubx/mga/updates";

// <Your PointPerfect Thing> -> Credentials -> Client Id
static const char MQTT_CLIENT_ID[] = "<ADD YOUR CLIENT ID HERE>";

// <Your PointPerfect Thing> -> Credentials -> Amazon Root Certificate
static const char AWS_CERT_CA[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
<ADD YOUR CERTICICATE HERE>
-----END CERTIFICATE-----)EOF";

// <Your PointPerfect Thing> -> Credentials -> Client Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(-----BEGIN CERTIFICATE-----
<ADD YOUR CERTICICATE HERE>
-----END CERTIFICATE-----)KEY";

// Get this from Thingstream Portal
// <Your PointPerfect Thing> -> Credentials -> Client Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(-----BEGIN RSA PRIVATE KEY-----
<ADD YOUR KEY HERE>
-----END RSA PRIVATE KEY-----)KEY";
