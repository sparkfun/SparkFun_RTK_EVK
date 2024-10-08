#include <HardwareSerial.h>
HardwareSerial laraSerial(1); // UART1

#include <SparkFun_u-blox_Cellular_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_u-blox_Cellular

// Derive the LARA_R6 class, so we can override beginSerial
class LARA_R6_Derived : public SparkFun_ublox_LARA_R6001D
{
public:
  LARA_R6_Derived() : SparkFun_ublox_LARA_R6001D{LARA_PWR} {} // Pass the LARA_PWR pin into the class so the library can powerOn / powerOff

protected:
  void beginSerial(unsigned long baud) override
  {
    delay(100);
    if (_hardSerial != nullptr)
    {
      _hardSerial->end();
      _hardSerial->begin(baud, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX); // Configure Serial1
    }
    delay(100);
  }
};

// Create a LARA_R6 object to use throughout the sketch
LARA_R6_Derived myLARA;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// helper macros to handle the AT interface errors
#define LTE_CHECK_INIT \
  int _step = 0;       \
  UBX_CELL_error_t _err = UBX_CELL_SUCCESS      //!< init variable
#define LTE_CHECK_OK (UBX_CELL_SUCCESS == _err) //!< record the return result
#define LTE_CHECK(x)            \
  if (UBX_CELL_SUCCESS == _err) \
  _step = x, _err //!< interim evaluate
#define LTE_CHECK_EVAL(_txt)                                                                   \
  if (UBX_CELL_SUCCESS != _err)                                                                \
    console->printf("%s: AT sequence failed at step %d with error %d\r\n", _txt, _step, _err); \
  else                                                                                         \
    console->printf("%s: AT sequence success\r\n", _txt)
#define LTE_CHECK_RETURN return (_err == UBX_CELL_SUCCESS)

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

const int LTE_HTTP_PROFILE = 0;          //!< The HTTP profile used duing provisioning
const int LTE_SEC_PROFILE_HTTP = 1;      //!< The security profile used for the HTTP connections druing provisioning
const int LTE_SEC_PROFILE_MQTT = 0;      //!< The security profile used for the MQTT connections
const char *FILE_REQUEST = "req.json";   //!< Temporarly file name used for the request during HTTP GET transations (ZTP)
const char *FILE_RESP = "resp.json";     //!< Temporarly file name used for the response during HTTP POST and GET transations (AWS/ZTP)
const char *SEC_ROOT_CA = "aws-rootCA";  //!< Temporarly file name used when injecting the ROOT CA
const char *SEC_CLIENT_CERT = "pp-cert"; //!< Temporarly file name used when injecting the client certificate
const char *SEC_CLIENT_KEY = "pp-key";   //!< Temporarly file name used when injecting the client keys

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/** The MQTT callback processes the URC and is responsible for advancing the state.
 *  \param command  the MQTT command
 *  \param request  the response code 1 = sucess, 0 = error
 */
void mqttCallback_LARA(int command, int result)
{
  console->printf("mqttCallback_LARA: command %d result %d\r\n", command, result);
  if (result == 0)
  {
    int code, code2;
    UBX_CELL_error_t err = myLARA.getMQTTprotocolError(&code, &code2);
    if (UBX_CELL_SUCCESS == err)
    {
      console->printf("mqttCallback_LARA: command %d protocol error code %d code2 %d\r\n", command, code, code2);
    }
    else
    {
      console->printf("mqttCallback_LARA: command %d protocol error failed with error %d\r\n", command, err);
    }
  }
  else
  {
    switch (command)
    {
    case UBX_CELL_MQTT_COMMAND_LOGIN:
    {
      console->println(F("mqttCallback_LARA: MQTT login"));
      mqttLogin = true;
    }
    break;
    case UBX_CELL_MQTT_COMMAND_LOGOUT:
    {
      console->println(F("mqttCallback_LARA: MQTT logout"));
      mqttLogin = false;
      mqttMsgs = 0;
      topics.clear();
      subTopic = "";
      unsubTopic = "";
      mqttFirstTime = true;
    }
    break;
    case UBX_CELL_MQTT_COMMAND_SUBSCRIBE:
    {
      if (!subTopic.length())
        console->printf("mqttCallback_LARA: MQTT subscribe result %d but no topic\r\n", result);
      else
      {
        console->printf("mqttCallback_LARA: MQTT subscribe result %d topic \"%s\"\r\n", result, subTopic.c_str());
        topics.push_back(subTopic);
        subTopic = "";
      }
    }
    break;
    case UBX_CELL_MQTT_COMMAND_UNSUBSCRIBE:
    {
      if (!unsubTopic.length())
        console->printf("mqttCallback_LARA: MQTT unsubscribe result %d but no topic\r\n", result);
      else
      {
        std::vector<String>::iterator pos = std::find(topics.begin(), topics.end(), unsubTopic);
        if (pos == topics.end()) // if unsubTopic is not in topics
        {
          console->printf("mqttCallback_LARA: MQTT unsubscribe result %d topic \"%s\" but topic not in list\r\n", result, unsubTopic.c_str());
          unsubTopic = ""; // Clear the unsubTopic otherwise mqttTask will always be busy
        }
        else
        {
          topics.erase(pos);
          console->printf("mqttCallback_LARA: MQTT unsubscribe result %d topic \"%s\"\r\n", result, unsubTopic.c_str());
          unsubTopic = "";
        }
      }
    }
    break;
    case UBX_CELL_MQTT_COMMAND_READ:
    {
      console->printf("mqttCallback_LARA: MQTT read result %d\r\n", result);
      mqttMsgs = result;
    }
    break;
    default:
      break;
    }
  }
}
//! static callback helper, regCallback will do the real work
static void mqttCallbackStatic_LARA(int command, int result)
{
  mqttCallback_LARA(command, result);
}

/** The HTTP callback processes the URC. clientID.length > 0 indicates the ZTP POST is complete
 *  \param profile  the HTTP profile
 *  \param command  the HTTP command
 *  \param request  the response code 1 = sucess, 0 = error
 */
void httpCallback_LARA(int profile, int command, int result)
{
  console->printf("profile %d command %d result %d\r\n", profile, command, result);
  if (result == 0)
  {
    int cls, code;
    UBX_CELL_error_t err = myLARA.getHTTPprotocolError(profile, &cls, &code);
    if (UBX_CELL_SUCCESS == err)
    {
      console->printf("profile %d command %d protocol error class %d code %d\r\n", profile, command, cls, code);
    }
    else
    {
      console->printf("profile %d command %d protocol error failed with error %d\r\n", profile, command, err);
    }
  }
  else if ((profile == LTE_HTTP_PROFILE) && ((command == UBX_CELL_HTTP_COMMAND_GET) ||
                                             (command == UBX_CELL_HTTP_COMMAND_POST_FILE)))
  {
    String str;
    LTE_CHECK_INIT;
    LTE_CHECK(1) = myLARA.getFileContents(FILE_RESP, &str);
    LTE_CHECK(2) = myLARA.deleteFile(FILE_RESP);
    LTE_CHECK_EVAL("HTTP read");
    if (LTE_CHECK_OK)
    {
      console->printf("HTTP ZTP response: %s\r\n", str.c_str());
          
      const char START_TAG[] = "\r\n\r\n";
      int offset = str.indexOf(START_TAG);
      if (offset)
      {
        str.remove(0, offset + sizeof(START_TAG) - 1);
        if (command == UBX_CELL_HTTP_COMMAND_POST_FILE)
        {
          // Device is now active with ThingStream
          // Pull pertinent values from response
          extractZTP(str);
        }
      }
    }
  }
}
//! static callback helper, regCallback will do the real work
static void httpCallbackStatic_LARA(int profile, int command, int result)
{
  httpCallback_LARA(profile, command, result);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/** Try to provision the PointPerfect to that we can start the MQTT server. This involves:
 *  HTTPS request to Thingstream POSTing the device token to get the credentials and client cert, key and ID
 */
void mqttProvision_LARA(void)
{
  String rootCa = AWS_CERT_CA;
  String ztpReq;
  ztpRequest(ztpReq);
  if (0 < ztpReq.length())
  {
    console->printf("HTTP ZTP connect to \"%s:%d\" and POST \"%s\"\r\n", THINGSTREAM_ZTPURL, HTTPS_PORT, ztpReq.c_str());
    myLARA.setHTTPCommandCallback(httpCallbackStatic_LARA); // callback will advance state
    LTE_CHECK_INIT;
    LTE_CHECK(1) = myLARA.setSecurityManager(UBX_CELL_SEC_MANAGER_OPCODE_IMPORT, UBX_CELL_SEC_MANAGER_ROOTCA, SEC_ROOT_CA, rootCa);
    LTE_CHECK(2) = myLARA.resetSecurityProfile(LTE_SEC_PROFILE_HTTP);
    LTE_CHECK(3) = myLARA.configSecurityProfile(LTE_SEC_PROFILE_HTTP, UBX_CELL_SEC_PROFILE_PARAM_CERT_VAL_LEVEL, UBX_CELL_SEC_PROFILE_CERTVAL_OPCODE_YESNOURL);
    LTE_CHECK(4) = myLARA.configSecurityProfile(LTE_SEC_PROFILE_HTTP, UBX_CELL_SEC_PROFILE_PARAM_TLS_VER, UBX_CELL_SEC_PROFILE_TLS_OPCODE_VER1_2);
    LTE_CHECK(5) = myLARA.configSecurityProfile(LTE_SEC_PROFILE_HTTP, UBX_CELL_SEC_PROFILE_PARAM_CYPHER_SUITE, UBX_CELL_SEC_PROFILE_SUITE_OPCODE_PROPOSEDDEFAULT);
    LTE_CHECK(6) = myLARA.configSecurityProfileString(LTE_SEC_PROFILE_HTTP, UBX_CELL_SEC_PROFILE_PARAM_ROOT_CA, SEC_ROOT_CA);
    LTE_CHECK(7) = myLARA.configSecurityProfileString(LTE_SEC_PROFILE_HTTP, UBX_CELL_SEC_PROFILE_PARAM_SNI, THINGSTREAM_SERVER);
    myLARA.deleteFile(FILE_REQUEST);                                // okay if this fails when file not present
    LTE_CHECK(8) = myLARA.appendFileContents(FILE_REQUEST, ztpReq); // now save the request, append to just deleted file
    LTE_CHECK(9) = myLARA.resetHTTPprofile(LTE_HTTP_PROFILE);
    LTE_CHECK(10) = myLARA.setHTTPserverName(LTE_HTTP_PROFILE, THINGSTREAM_SERVER);
    LTE_CHECK(11) = myLARA.setHTTPserverPort(LTE_HTTP_PROFILE, HTTPS_PORT); // make sure port is set
    LTE_CHECK(12) = myLARA.setHTTPauthentication(LTE_HTTP_PROFILE, false);
    LTE_CHECK(13) = myLARA.setHTTPsecure(LTE_HTTP_PROFILE, true, LTE_SEC_PROFILE_HTTP);
    LTE_CHECK(14) = myLARA.sendHTTPPOSTfile(LTE_HTTP_PROFILE, THINGSTREAM_ZTPPATH, FILE_RESP, FILE_REQUEST, UBX_CELL_HTTP_CONTENT_APPLICATION_JSON);
    LTE_CHECK_EVAL("mqttProvision_LARA");
  }

  unsigned long ztpStart = millis();

  // Wait for the ZTP to complete. clientID will be updated via the HTTP callback
  while ((clientID.length() == 0) && (millis() < (ztpStart + HTTP_CONNECT_TIMEOUT_MS)))
  {
    myLARA.bufferedPoll(); // Process any URC messages from the LARA
  }
}

/** Connect to the Thingstream PointPerfect server using the credentials from ZTP process
 *  \param id  the client ID for this device
 */
void mqttConnect_LARA(void)
{
  // disconncect must fail here, so that we can connect
  myLARA.setMQTTCommandCallback(mqttCallbackStatic_LARA); // callback will advance state
  // make sure the client is disconnected here
  if (UBX_CELL_SUCCESS == myLARA.disconnectMQTT())
  {
    console->println("mqttConnect_LARA: forced disconnect"); // if this sucessful it means were were still connected.
  }
  else
  {
    console->printf("mqttConnect_LARA: connecting to \"%s:%d\" as client \"%s\"\r\n", AWS_IOT_ENDPOINT, AWS_IOT_PORT, clientID.c_str());
    LTE_CHECK_INIT;
    LTE_CHECK(1) = myLARA.setSecurityManager(UBX_CELL_SEC_MANAGER_OPCODE_IMPORT, UBX_CELL_SEC_MANAGER_ROOTCA, String(SEC_ROOT_CA), String(AWS_CERT_CA));
    LTE_CHECK(2) = myLARA.setSecurityManager(UBX_CELL_SEC_MANAGER_OPCODE_IMPORT, UBX_CELL_SEC_MANAGER_CLIENT_CERT, SEC_CLIENT_CERT, clientCert);
    LTE_CHECK(3) = myLARA.setSecurityManager(UBX_CELL_SEC_MANAGER_OPCODE_IMPORT, UBX_CELL_SEC_MANAGER_CLIENT_KEY, SEC_CLIENT_KEY, clientKey);
    LTE_CHECK(4) = myLARA.resetSecurityProfile(LTE_SEC_PROFILE_MQTT);
    LTE_CHECK(5) = myLARA.configSecurityProfile(LTE_SEC_PROFILE_MQTT, UBX_CELL_SEC_PROFILE_PARAM_CERT_VAL_LEVEL, UBX_CELL_SEC_PROFILE_CERTVAL_OPCODE_YESNOURL);
    LTE_CHECK(6) = myLARA.configSecurityProfile(LTE_SEC_PROFILE_MQTT, UBX_CELL_SEC_PROFILE_PARAM_TLS_VER, UBX_CELL_SEC_PROFILE_TLS_OPCODE_VER1_2);
    LTE_CHECK(7) = myLARA.configSecurityProfile(LTE_SEC_PROFILE_MQTT, UBX_CELL_SEC_PROFILE_PARAM_CYPHER_SUITE, UBX_CELL_SEC_PROFILE_SUITE_OPCODE_PROPOSEDDEFAULT);
    LTE_CHECK(8) = myLARA.configSecurityProfileString(LTE_SEC_PROFILE_MQTT, UBX_CELL_SEC_PROFILE_PARAM_ROOT_CA, SEC_ROOT_CA);
    LTE_CHECK(9) = myLARA.configSecurityProfileString(LTE_SEC_PROFILE_MQTT, UBX_CELL_SEC_PROFILE_PARAM_CLIENT_CERT, SEC_CLIENT_CERT);
    LTE_CHECK(10) = myLARA.configSecurityProfileString(LTE_SEC_PROFILE_MQTT, UBX_CELL_SEC_PROFILE_PARAM_CLIENT_KEY, SEC_CLIENT_KEY);
    LTE_CHECK(11) = myLARA.configSecurityProfileString(LTE_SEC_PROFILE_MQTT, UBX_CELL_SEC_PROFILE_PARAM_SNI, AWS_IOT_ENDPOINT);
    LTE_CHECK(12) = myLARA.nvMQTT(UBX_CELL_MQTT_NV_RESTORE);
    LTE_CHECK(13) = myLARA.setMQTTclientId(clientID);
    LTE_CHECK(14) = myLARA.setMQTTserver(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
    LTE_CHECK(15) = myLARA.setMQTTsecure(true, LTE_SEC_PROFILE_MQTT);
    LTE_CHECK(16) = myLARA.connectMQTT();
    LTE_CHECK_EVAL("mqttConnect_LARA");
    mqttMsgs = 0;
    topics.clear();
    subTopic = "";
    unsubTopic = "";
    mqttFirstTime = true;
  }
}

/** Disconnect and cleanup the MQTT connection
 *  \return true if already disconnected (no need to wait for the callback)
 */
bool mqttStop_LARA(void)
{
  UBX_CELL_error_t err = myLARA.disconnectMQTT();
  if (UBX_CELL_SUCCESS == err)
  {
    console->println("mqttStop_LARA: disconnect");
  }
  else
  {
    console->printf("mqttStop_LARA: disconnect failed with error %d\r\n", err);
  }
  return UBX_CELL_SUCCESS != err;
}

/** The MQTT task is responsible for:
 *  1) subscribing to topics
 *  2) unsubscribing from topics
 *  3) read MQTT data from the modem and inject it into the GNSS receiver
 */
void mqttTask_LARA(bool keyPress)
{
  if (keyPress) // Has the user pressed a key?
  {
    if (!mqttLogin)
      mqttConnect_LARA();
    else
      mqttStop_LARA();
    return; // Return now
  }

  myLARA.bufferedPoll(); // Process any URC messages from the LARA

  static unsigned long lastTask = millis();
  if (millis() > (lastTask + mqttTaskInterval)) // Perform the task every mqttTaskInterval ms
    lastTask = millis();
  else
    return;

  if (!mqttLogin) // We can only subscribe to topics and read MQTT data when connected
    return;

  /* The LTE modem has difficulties subscribing/unsubscribing more than one topic at the same time
   * We can only start one operation at a time wait for the URC and add a extra delay before we can
   * do the next operation.
   */
  bool busy = (0 < subTopic.length()) || (0 < unsubTopic.length());
  if (!busy)
  {
    std::vector<String> newTopics;
    newTopics.clear();
    newTopics.push_back(MQTT_TOPIC_KEY); // Always subscribe to the keys topic
    if ((myLat == 99999) || (myLon == 99999) || !useLocalizedDistribution)
      newTopics.push_back(MQTT_TOPIC_SPARTN); // If our location is unknown, subscribe to the full continental topic
    else
    {
      if ((myLat != lastLat) || (myLon != lastLon) || !tileKnown) // Update tile topic if position has changed
      {
        lastLat = myLat;
        lastLon = myLon;
        tileKnown = false;
        char verifiedLevel = localizedLevel;
        if ((verifiedLevel < '0') || (verifiedLevel > '2'))
          verifiedLevel = '2';
        snprintf(tileTopic, sizeof(tileTopic), "%s%c%c%04d%c%05d/dict", localizedPrefix, verifiedLevel, (myTileLat < 0) ? 'S' : 'N', abs(myTileLat), (myTileLon) < 0 ? 'W' : 'E', abs(myTileLon));
        newTopics.push_back(tileTopic);         // Subscribe to the nearest localized topic
        newTopics.push_back(MQTT_TOPIC_SPARTN); // We still need the full continental topic until we have our local tile
      }
      else
        newTopics.push_back(tileTopic); // Tile is known so subscribe to only our localized topic
    }
    if (useAssistNow)
    {
      if (mqttFirstTime || !useAssistNowUpdates) // First time, subscribe to the full Assist Now MGA data, thereafter subscribe to updates only
        newTopics.push_back(MQTT_TOPIC_ASSISTNOW);
      else
        newTopics.push_back(MQTT_TOPIC_ASSISTNOW_UPDATES);
    }

    // loop through new topics and subscribe to the first topic that is not in our curent topics list.
    for (auto it = newTopics.begin(); (it != newTopics.end()) && !busy; it = std::next(it))
    {
      String topic = *it;
      std::vector<String>::iterator pos = std::find(topics.begin(), topics.end(), topic);
      if (pos == topics.end()) // if topic from newTopics is not in topics
      {
        UBX_CELL_error_t err = myLARA.subscribeMQTTtopic(0, topic);
        if (UBX_CELL_SUCCESS == err)
        {
          console->printf("mqttTask: subscribe request topic \"%s\" qos %d\r\n", topic.c_str(), 0);
          subTopic = topic;
        }
        else
        {
          console->printf("mqttTask: subscribe request topic \"%s\" qos %d failed with error %d\r\n", topic.c_str(), 0, err);
        }
        busy = true;
      }
    }
    // loop through current topics and unsubscribe to the first topic that is not in the new topics list.
    for (auto it = topics.begin(); (it != topics.end()) && !busy; it = std::next(it))
    {
      String topic = *it;
      std::vector<String>::iterator pos = std::find(newTopics.begin(), newTopics.end(), topic);
      if (pos == newTopics.end()) // if topic from topics is not in newTopics
      {
        UBX_CELL_error_t err = myLARA.unsubscribeMQTTtopic(topic);
        if (UBX_CELL_SUCCESS == err)
        {
          console->printf("mqttTask: unsubscribe requested topic \"%s\"\r\n", topic.c_str());
          unsubTopic = topic;
        }
        else
        {
          console->printf("mqttTask: unsubscribe request topic \"%s\" failed with error %d\r\n", topic.c_str(), err);
        }
        busy = true;
      }
    }
    if (!busy && (0 < mqttMsgs))
    {
      // at this point we are properly subscribed to the needed topics and can now read data
      console->printf("mqttTask: read request %d msg\r\n", mqttMsgs);
      // The MQTT API does not allow getting the size before actually reading the data. So we
      // have to allocate a big enough buffer. PointPerfect may send upto 9kB on the MGA topic.
      uint8_t *buf = new uint8_t[MQTT_MAX_MSG_SIZE];
      if (buf != NULL)
      {
        String topic;
        int len = -1;
        int qos = -1;
        UBX_CELL_error_t err = myLARA.readMQTT(&qos, &topic, buf, MQTT_MAX_MSG_SIZE, &len);
        if (UBX_CELL_SUCCESS == err)
        {
          mqttMsgs = 0; // expect a URC afterwards
          const char *strTopic = topic.c_str();
          console->printf("mqttTask: topic \"%s\" read %d bytes\r\n", strTopic, len);
          // if we detect data from a topic, then why not unsubscribe from it.
          std::vector<String>::iterator pos = std::find(topics.begin(), topics.end(), topic);
          if (pos == topics.end()) // if topic from MQTT is not in topics
          {
            console->printf("mqttTask: getting data from an unexpected topic \"%s\"\r\n", strTopic);
            if (!busy)
            {
              err = myLARA.unsubscribeMQTTtopic(topic);
              if (UBX_CELL_SUCCESS == err)
              {
                console->printf("mqttTask: unsubscribe request for unexpected topic \"%s\"\r\n", strTopic);
                unsubTopic = topic;
              }
              else
              {
                console->printf("mqttTask: unsubscribe request for unexpected topic \"%s\" failed with error %d\r\n", topic.c_str(), err);
              }
              busy = true;
            }
          }
          else if ((topic.equals(tileTopic)) && (strstr(strTopic, "/dict") != nullptr)) // Check if this is a dictionary of tile nodes
          {
            console->println("mqttTask: localized distribution dict received");
            // This is a cheat... We should be using a JSON library to read the nodes:
            // {
            //   "tile": "L2N5375W00125",
            //   "nodeprefix": "pp/ip/L2N5375W00125/",
            //   "nodes": [
            //     "N5200W00300",
            //     "N5200W00200",
            //     "N5200W00100",
            //     "N5200E00000",
            //     "N5200E00100",
            //     "N5300W00300",
            //     "N5300W00200",
            //     "N5300W00100",
            //     "N5300E00000",
            //     "N5400W00200",
            //     "N5400W00100",
            //     "N5500W00300",
            //     "N5500W00200"
            //   ],
            //   "endpoint": "pp-eu.services.u-blox.com"
            // }
            // But life is short... Here we check the nodes using sscanf.
            char *nodes = strstr((const char *)buf, "\"nodes\":[");
            if (nodes != nullptr)
            {
              nodes += strlen("\"nodes\":["); // Point to the first node
              float minDist = 99999.0;        // Minimum distance to tile center in centidegrees
              char *preservedTile;
              char *tile = strtok_r(nodes, ",", &preservedTile);
              while (tile != nullptr)
              {
                char ns, ew;
                int lat, lon;
                if (sscanf(tile, "\"%c%d%c%d\"", &ns, &lat, &ew, &lon) == 4)
                {
                  if (ns == 'S')
                    lat = 0 - lat;
                  if (ew == 'W')
                    lon = 0 - lon;
                  float factorLon = cos(radians((float)myLat / 100.0));                                 // Scale lon by the lat
                  float distScaled = pow(pow(lat - myLat, 2) + pow((lon - myLon) * factorLon, 2), 0.5); // Calculate distance to tile center in centidegrees
                  if (distScaled < minDist)
                  {
                    minDist = distScaled;
                    tile[12] = 0; // Convert the second quote to NULL for snprintf
                    tileKnown = true;
                    snprintf(&tileTopic[strlen(localizedPrefix) + 13], sizeof(tileTopic) - (strlen(localizedPrefix) + 13), "%s", tile + 1); // Start after the first quote
                  }
                }
                tile = strtok_r(nullptr, ",", &preservedTile);
              }
            }
          }
          else
          {
            // Anything else can be sent to the GNSS as is
            myGNSS.pushRawData(buf, (size_t)len);
            console->printf("mqttTask: pushing %d bytes to the GNSS\r\n", len);

            // Check if pushing the full AssistNow MGA data
            if (topic.equals(MQTT_TOPIC_ASSISTNOW))
              mqttFirstTime = false;
          }
        }
        else
        {
          console->printf("mqttTask: read failed with error %d\r\n", err);
        }
        // we need to free the buffer, as the inject function took a copy with only the required size
        delete[] buf;
      }
    }
  }

  lastTask = millis(); // Update lastTask so we wait mqttTaskInterval from now
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool initLARA(void)
{
  // The LARA_R6 will be powered off by default.
  // If desired, we can power it on manually by toggling the LARA_PWR pin now.
  // Or we can wait and let myLARA.begin do it.
  // {
  //   digitalWrite(LARA_PWR, HIGH);
  //   delay(100);
  //   digitalWrite(LARA_PWR, LOW);

  //   delay(8000); // Wait > 7 seconds for the LARA to begin
  // }

  // myLARA.enableDebugging(*console); // Uncomment this line to show debug messages on the console

  myLARA.invertPowerPin(true); // LARA_PWR is inverted

  console->println(F("Initializing the LARA_R6. This could take ~20 seconds"));

  // Initialize the LARA
  if (myLARA.begin(laraSerial, 115200))
  {
    console->println(F("LARA-R6 connected"));
  }
  else
  {
    console->println(F("Unable to communicate with the LARA!"));
    return false;
  }

  console->print(F("Waiting for NI to go high"));
  int tries = 0;
  const int maxTries = 60;
  while ((tries < maxTries) && (digitalRead(LARA_NI) == LOW))
  {
    delay(1000);
    console->print(F("."));
    tries++;
  }
  console->println();
  if (tries == maxTries)
  {
    console->println(F("NI didn't go high!"));
    return false;
  }

  String currentOperator = "";

  // First check to see if we're connected to an operator:
  if (myLARA.getOperator(&currentOperator) == UBX_CELL_SUCCESS)
  {
    console->print(F("Connected to: "));
    console->println(currentOperator);
  }
  else
  {
    console->print(F("The LARA is not yet connected to an operator!"));
    return false;
  }

  return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
