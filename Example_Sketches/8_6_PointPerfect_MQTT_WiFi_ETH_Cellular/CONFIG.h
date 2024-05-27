const uint16_t HTTPS_PORT = 443; //!< The HTTPS default port

String clientCert = ""; // This should be stored in LittleFS
String clientKey = ""; // This should be stored in LittleFS
String clientID = ""; // This should be stored in LittleFS
String brokerHost = ""; // This should be stored in LittleFS
String currentKey = ""; // This should be stored in LittleFS
String currentKeyDuration = ""; // This should be stored in LittleFS
String currentKeyStart = ""; // This should be stored in LittleFS
String nextKey = ""; // This should be stored in LittleFS
String nextKeyDuration = ""; // This should be stored in LittleFS
String nextKeyStart = ""; // This should be stored in LittleFS

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
    console->printf("ZTP request %s\r\n", str.c_str());
  }
}
