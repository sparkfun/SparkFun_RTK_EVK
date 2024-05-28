// Settings for ZTP request that use HTTPS protocol
#define THINGSTREAM_SERVER "api.thingstream.io"                                      //!< the thingstream Rest API server domain
#define THINGSTREAM_ZTPPATH "/ztp/pointperfect/credentials"                          //!< ZTP rest api
const char THINGSTREAM_ZTPURL[] = "https://" THINGSTREAM_SERVER THINGSTREAM_ZTPPATH; // full ZTP url

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
const char localizedPrefix[] = "pp/ip/L";   // The localized distribution topic prefix. Note: starts with "pp", not "/pp"
const char localizedLevel = '2';            // The localized distribution level: 0 = 10 degrees; 1 = 5 degrees; 2 = 2.5 degrees

// <Your PointPerfect Thing> -> Credentials -> AssistNow (MGA) topic
const bool useAssistNow = true; // Set to true to use Assist Now MGA
const char MQTT_TOPIC_ASSISTNOW[] = "/pp/ubx/mga";
const bool useAssistNowUpdates = true; // If useAssistNow is true, set to true to use updates (less traffic)
const char MQTT_TOPIC_ASSISTNOW_UPDATES[] = "/pp/ubx/mga/updates";

const uint16_t HTTPS_PORT = 443; //!< The HTTPS default port

const unsigned long HTTP_CONNECT_TIMEOUT_MS = 10000; // ms

String clientCert = "";         // This should be stored in LittleFS
String clientKey = "";          // This should be stored in LittleFS
String clientID = "";           // This should be stored in LittleFS
String brokerHost = "";         // This should be stored in LittleFS
String currentKey = "";         // This should be stored in LittleFS
String currentKeyDuration = ""; // This should be stored in LittleFS
String currentKeyStart = "";    // This should be stored in LittleFS
String nextKey = "";            // This should be stored in LittleFS
String nextKeyDuration = "";    // This should be stored in LittleFS
String nextKeyStart = "";       // This should be stored in LittleFS

std::vector<String> topics; //!< vector with current subscribed topics
String subTopic = "";       //!< requested topic to be subscribed (needed by the callback)
String unsubTopic = "";     //!< requested topic to be un-subscribed (needed by the callback)
int mqttMsgs;               //!< remember the number of messages pending indicated by the URC
bool mqttLogin = false;     // Remember if we are connected to MQTT
bool mqttFirstTime = true;  // Remember if this is the first connection, so we can request MGA

const unsigned long mqttTaskInterval = 100; // Minimum interval between MQTT task calls - needed for subscribe/unsubscribe

const int MQTT_MAX_MSG_SIZE = 10 * 1024; //!< the max size of a MQTT pointperfect topic

int lastLat = 99999; // The last lat and lon in centidegrees. Used to re-subscribe to the localized distribution topics
int lastLon = 99999;
char tileTopic[40] = {0}; // Contains the localized distribution topic: ends with /dict initially, then the nearest tile
bool tileKnown = false;   // Set to true when the nearest tile is known

/** create a ZTP request to be sent to thingstream JSON API
 *  \return  the ZTP request string to POST
 */
void ztpRequest(String &str)
{
  str = "";
  DynamicJsonDocument json(256);
  json["tags"][0] = "ztp";
  json["token"] = POINTPERFECT_TOKEN;
  json["hardwareId"] = ZTP_HARDWARE_ID;
  json["givenName"] = ZTP_GIVEN_NAME;
  if (0 < serializeJson(json, str))
  {
    console->printf("ZTP request: %s\r\n", str.c_str());
  }
}

void extractZTP(String &str)
{
  // Pull pertinent values from response
  DynamicJsonDocument *jsonZtp = new DynamicJsonDocument(8192);
  if (!jsonZtp)
  {
    console->println("ERROR - Failed to allocate jsonZtp!");
  }
  else
  {
    DeserializationError error = deserializeJson(*jsonZtp, str);
    if (DeserializationError::Ok != error)
    {
      console->println("JSON deserialize error");
    }
    else
    {
      clientCert = (const char *)((*jsonZtp)["certificate"]);

      clientKey = (const char *)((*jsonZtp)["privateKey"]);

      // TODO: checkCertificates

      clientID = (const char *)((*jsonZtp)["clientId"]);

      brokerHost = (const char *)((*jsonZtp)["brokerHost"]);

      // TODO: store the key distribution topic and correction topics

      currentKey = (const char *)((*jsonZtp)["dynamickeys"]["current"]["value"]);
      currentKeyDuration = (const char *)(*jsonZtp)["dynamickeys"]["current"]["duration"];
      currentKeyStart = (const char *)(*jsonZtp)["dynamickeys"]["current"]["start"];

      nextKey = (const char *)((*jsonZtp)["dynamickeys"]["next"]["value"]);
      nextKeyDuration = (const char *)(*jsonZtp)["dynamickeys"]["next"]["duration"];
      nextKeyStart = (const char *)(*jsonZtp)["dynamickeys"]["next"]["start"];
    } // JSON Derialized correctly
  }
}