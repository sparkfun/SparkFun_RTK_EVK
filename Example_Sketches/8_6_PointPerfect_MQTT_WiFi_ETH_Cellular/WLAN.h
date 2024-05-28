#include <WiFi.h>
#include <WiFiMulti.h>
#include <NetworkClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoMqttClient.h>

WiFiMulti wifiMulti;
NetworkClientSecure lanClient;
HTTPClient httpClient;
MqttClient mqttClient(lanClient);

// Initialize WiFi
bool initWLAN()
{
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD); // SSID and PASSWORD from secrets.h

  console->println("Connecting Wifi...");

  if (wifiMulti.run() == WL_CONNECTED) {
    console->println("");
    console->println("WiFi connected");
    console->print("IP address: ");
    console->println(WiFi.localIP());
    return true;
  }

  console->println("WiFi connect failed!");
  
  return false;
}

/** Try to provision the PointPerfect to that we can start the MQTT server. This involves: 
 *  HTTPS request to Thingstream POSTing the device token to get the credentials and client cert, key and ID
 */
void mqttProvision_LAN(void) {
  String ztpReq;
  ztpRequest(ztpReq);
  if (0 < ztpReq.length()) {
    // Perform PointPerfect ZTP
    lanClient.setCACert(AWS_CERT_CA);

    if (!lanClient.connect(THINGSTREAM_SERVER, HTTPS_PORT))
    {
      console->printf("lanClient.connect to %s on port %d failed!\r\n", THINGSTREAM_SERVER, HTTPS_PORT);
    }
    else
    {
      console->printf("lanClient connected to %s on port %d\r\n", THINGSTREAM_SERVER, HTTPS_PORT);

      httpClient.begin(THINGSTREAM_ZTPURL);
      httpClient.setConnectTimeout(5000);
      httpClient.addHeader(F("Content-Type"), F("application/json"));
      console->printf("HTTP ZTP \"%s:%d\" POST \"%s\"\r\n", THINGSTREAM_ZTPURL, HTTPS_PORT, ztpReq.c_str());
      int httpResponseCode = httpClient.POST(ztpReq.c_str());
      String str = httpClient.getString();
      httpClient.end();
      if (httpResponseCode != HTTP_CODE_OK) {
        console->printf("HTTP ZTP response error %d %s\r\n", httpResponseCode, str.c_str());
      } else {
        console->printf("HTTP ZTP response: %s\r\n", str.c_str());

        // Pull pertinent values from response
        extractZTP(str);
      }

      lanClient.stop();
    }
  }
}

// -----------------------------------------------------------------------
// MQTT / PointPerfect
// -----------------------------------------------------------------------

/** The MQTT callback processes is responsible for reading the data
 *  \param messageSize the bytes pending top be read
 */
void onMQTT_LAN(int messageSize) {
  if (messageSize) {    
    uint8_t *buf = new uint8_t[messageSize];
    if (buf != nullptr)
    {
      String topic = mqttClient.messageTopic();
      int len = mqttClient.read(buf, messageSize);

      if (len == messageSize)
      {
        const char *strTopic = topic.c_str();
        console->printf("onMQTT_LAN: topic \"%s\" read %d bytes\r\n", strTopic, len);

        // If we detect data from an unexpected topic, we could unsubscribe from it.
        // But the server may just be being slow to respond to a previous unsubscribe...
        /*
        std::vector<String>::iterator pos = std::find(topics.begin(), topics.end(), topic);
        if (pos == topics.end()) // if topic from MQTT is not in topics
        {
          console->printf("onMQTT_LAN: getting data from an unexpected topic \"%s\"\r\n", strTopic);
          if (mqttClient.unsubscribe(topic))
          {
            console->printf("onMQTT_LAN: unsubscribe request for unexpected topic \"%s\"\r\n", strTopic);
          }
          else
          {
            console->printf("onMQTT_LAN: unsubscribe request for unexpected topic \"%s\" failed!\r\n", topic.c_str());
          }
        }
        else
        */
        if ((topic.equals(tileTopic)) && (strstr(strTopic, "/dict") != nullptr)) // Check if this is a dictionary of tile nodes
        {
          console->println("onMQTT_LAN: localized distribution dict received");
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
          console->printf("onMQTT_LAN: pushing %d bytes to the GNSS\r\n", len);

          // Check if pushing the full AssistNow MGA data
          if (topic.equals(MQTT_TOPIC_ASSISTNOW))
            mqttFirstTime = false;
        }
      }
      else
      {
        console->printf("onMQTT_LAN: read failed! Read %d bytes, expected %d bytes\r\n", len, messageSize);
      }

      delete[] buf;
    }
    else
    {
      console->println("onMQTT_LAN: buf memory allocation failed!");
    }
  }
}
//! static callback helper, onMQTT will do the real work
static void onMQTTStatic_LAN(int messageSize) {
  onMQTT_LAN(messageSize);
}

/** Connect to the Thingstream PointPerfect server using the credentials from ZTP process
 *  \param id  the client ID for this device
 */
bool mqttConnect_LAN() {
  lanClient.setCACert(AWS_CERT_CA);
  lanClient.setCertificate(clientCert.c_str());
  lanClient.setPrivateKey(clientKey.c_str());
  mqttClient.setId(clientID.c_str());
  mqttClient.onMessage(onMQTTStatic_LAN);
  mqttClient.setKeepAliveInterval(60 * 1000);
  mqttClient.setConnectionTimeout( 5 * 1000);
  if (mqttClient.connect(brokerHost.c_str(), AWS_IOT_PORT)) {
    console->printf("mqttConnect_LAN: server \"%s:%d\" as client \"%s\"\r\n", brokerHost.c_str(), AWS_IOT_PORT, clientID.c_str());
  } else {
    int err = mqttClient.connectError(); 
    const char* LUT[] = { "REFUSED", "TIMEOUT", "OK", "PROT VER", "ID BAD", "SRV NA", "BAD USER/PWD", "NOT AUTH" };
    console->printf("mqttConnect_LAN: server \"%s\":%d as client \"%s\" failed with error %d(%s)\r\n",
              brokerHost.c_str(), AWS_IOT_PORT, clientID.c_str(), err, LUT[err + 2]);
  }
  topics.clear();
  mqttFirstTime = true;
  return mqttClient.connected();
}

/** Disconnect and cleanup the MQTT connection
 */
void mqttStop_LAN(void) {
  for (auto it = topics.begin(); it != topics.end(); it = std::next(it)) {
    String topic = *it;
    console->printf("mqttStop: unsubscribe \"%s\"\r\n", topic.c_str());
    mqttClient.unsubscribe(topic);
  }
  topics.clear();
  if (mqttClient.connected()) {
    console->println("mqttStop: disconnect");
    mqttClient.stop();
  }
}

/** The MQTT task is responsible for:
 *  1) subscribing to topics
 *  2) unsubscribing from topics 
 */
void mqttTask_LAN(bool keyPress) {
  if (keyPress) // Has the user pressed a key?
  {
    if (!mqttClient.connected())
      mqttConnect_LAN();
    else
      mqttStop_LAN();
    return; // Return now
  }

  static unsigned long lastTask = millis();
  if (millis() > (lastTask + mqttTaskInterval)) // Perform the task every mqttTaskInterval ms
    lastTask = millis();
  else
    return;

  if (!mqttClient.connected()) // We can only subscribe to topics and read MQTT data when connected
    return;

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

  // loop through new topics and subscribe to any that are not in our curent topics list.
  for (auto it = newTopics.begin(); (it != newTopics.end()); it = std::next(it))
  {
    String topic = *it;
    std::vector<String>::iterator pos = std::find(topics.begin(), topics.end(), topic);
    if (pos == topics.end()) // if topic from newTopics is not in topics
    {
      if (mqttClient.subscribe(topic))
      {
        console->printf("mqttTask_LAN: subscribe request topic \"%s\"\r\n", topic.c_str());
        topics.push_back(topic);
      }
      else
      {
        console->printf("mqttTask_LAN: subscribe request topic \"%s\" failed!\r\n", topic.c_str());
      }
    }
  }
  // loop through current topics and unsubscribe from any that are not in the new topics list.
  for (auto it = topics.begin(); (it != topics.end()); it = std::next(it))
  {
    String topic = *it;
    std::vector<String>::iterator pos = std::find(newTopics.begin(), newTopics.end(), topic);
    if (pos == newTopics.end()) // if topic from topics is not in newTopics
    {
      if (mqttClient.unsubscribe(topic))
      {
        console->printf("mqttTask_LAN: unsubscribe requested topic \"%s\"\r\n", topic.c_str());
        topics.erase(it);
      }
      else
      {
        console->printf("mqttTask_LAN: unsubscribe request topic \"%s\" failed!\r\n", topic.c_str());
      }
    }
  }

  lastTask = millis(); // Update lastTask so we wait mqttTaskInterval from now
}
