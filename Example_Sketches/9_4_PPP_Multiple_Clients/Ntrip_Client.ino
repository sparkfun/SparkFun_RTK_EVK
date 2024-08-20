/*------------------------------------------------------------------------------
Ntrip_Client.ino

  The NTRIP client sits on top of the network layer and receives correction data
  from an NTRIP caster that is provided to the ZED (GNSS radio).

                Satellite     ...    Satellite
                     |         |          |
                     |         |          |
                     |         V          |
                     |        RTK         |
                     '------> Base <------'
                            Station
                               |
                               | NTRIP Server sends correction data
                               V
                         NTRIP Caster
                               |
                               | NTRIP Client receives correction data
                               V
            Bluetooth         RTK        Network: NMEA Client
           .---------------- Rover --------------------------.
           |                   |                             |
           | NMEA              | Network: NEMA Server        | NMEA
           | position          | NEMA position data          | position
           | data              V                             | data
           |              Computer or                        |
           '------------> Cell Phone <-----------------------'
                          for display

  Possible NTRIP Casters

    * https://emlid.com/ntrip-caster/
    * http://rtk2go.com/
    * private SNIP NTRIP caster
------------------------------------------------------------------------------*/

//----------------------------------------
// Constants
//----------------------------------------

// Size of the credentials buffer in bytes
static const int CREDENTIALS_BUFFER_SIZE = 512;

// Give up connecting after this number of attempts
// Connection attempts are throttled to increase the time between attempts
// 30 attempts with 15 second increases will take almost two hours
static const int MAX_NTRIP_CLIENT_CONNECTION_ATTEMPTS = 30;

// NTRIP caster response timeout
static const uint32_t NTRIP_CLIENT_RESPONSE_TIMEOUT = 10 * 1000; // Milliseconds

// NTRIP client receive data timeout
static const uint32_t NTRIP_CLIENT_RECEIVE_DATA_TIMEOUT = 60 * 1000; // Milliseconds

// Most incoming data is around 500 bytes but may be larger
static const int RTCM_DATA_SIZE = 512 * 4;

// NTRIP client server request buffer size
static const int SERVER_BUFFER_SIZE = CREDENTIALS_BUFFER_SIZE + 3;

static const int NTRIPCLIENT_MS_BETWEEN_GGA = 5000; // 5s between transmission of GGA messages, if enabled

// NTRIP client connection delay before resetting the connect accempt counter
static const int NTRIP_CLIENT_CONNECTION_TIME = 5 * 60 * 1000;

// Define the NTRIP client states
enum NTRIPClientState
{
    NTRIP_CLIENT_NETWORK_STARTED = 0, // Connecting to WiFi access point or Ethernet
    NTRIP_CLIENT_NETWORK_CONNECTED, // Connected to an access point or Ethernet
    NTRIP_CLIENT_CONNECTING,        // Attempting a connection to the NTRIP caster
    NTRIP_CLIENT_WAIT_RESPONSE,     // Wait for a response from the NTRIP caster
    NTRIP_CLIENT_CONNECTED,         // Connected to the NTRIP caster
    NTRIP_CLIENT_OFF,
    // Insert new states here
    NTRIP_CLIENT_STATE_MAX // Last entry in the state list
};

const char *const ntripClientStateName[] = {"NTRIP_CLIENT_NETWORK_STARTED",
                                            "NTRIP_CLIENT_NETWORK_CONNECTED",
                                            "NTRIP_CLIENT_CONNECTING",
                                            "NTRIP_CLIENT_WAIT_RESPONSE",
                                            "NTRIP_CLIENT_CONNECTED"};

const int ntripClientStateNameEntries = sizeof(ntripClientStateName) / sizeof(ntripClientStateName[0]);

//----------------------------------------
// Locals
//----------------------------------------

// The network connection to the NTRIP caster to obtain RTCM data.
static NetworkClient *ntripClient;
static volatile uint8_t ntripClientState;

// NTRIP client timer usage:
//  * Reconnection delay
//  * Measure the connection response time
//  * Receive NTRIP data timeout
static uint32_t ntripClientTimer;

// Throttle GGA transmission to Caster to 1 report every 5 seconds
unsigned long lastGGAPush;

//----------------------------------------
// NTRIP Client Routines
//----------------------------------------

bool ntripClientConnect()
{
    if (!ntripClient)
        return false;

    if (debugNtripClientState)
        Serial.printf("NTRIP Client connecting to %s:%d\r\n", casterHost,
                     casterPort);

    int connectResponse = ntripClient->connect(casterHost, casterPort);

    if (connectResponse < 1)
    {
        if (debugNtripClientState)
            Serial.printf("NTRIP Client connection to NTRIP caster %s:%d failed\r\n", casterHost,
                         casterPort);
        return false;
    }

    // Set up the server request (GET)
    char serverRequest[SERVER_BUFFER_SIZE];
    int length;
    snprintf(serverRequest, SERVER_BUFFER_SIZE, "GET /%s HTTP/1.0\r\nUser-Agent: NTRIP SparkFun_RTK_%s_",
             mountPoint, platformPrefix);
    length = strlen(serverRequest);
    serverRequest[length++] = '\r';
    serverRequest[length++] = '\n';
    serverRequest[length++] = 0;

    // Set up the credentials
    char credentials[CREDENTIALS_BUFFER_SIZE];
    if (strlen(casterUser) == 0)
    {
        strncpy(credentials, "Accept: */*\r\nConnection: close\r\n", sizeof(credentials) - 1);
    }
    else
    {
        // Pass base64 encoded user:pw
        char userCredentials[sizeof(casterUser) + sizeof(casterUserPW) +
                             1]; // The ':' takes up a spot
        snprintf(userCredentials, sizeof(userCredentials), "%s:%s", casterUser,
                 casterUserPW);

        if (debugNtripClientState)
        {
            Serial.print("NTRIP Client sending credentials: ");
            Serial.println(userCredentials);
        }

        // Encode with ESP32 built-in library
        base64 b;
        String strEncodedCredentials = b.encode(userCredentials);
        char encodedCredentials[strEncodedCredentials.length() + 1];
        strEncodedCredentials.toCharArray(encodedCredentials,
                                          sizeof(encodedCredentials)); // Convert String to char array

        snprintf(credentials, sizeof(credentials), "Authorization: Basic %s\r\n", encodedCredentials);
    }

    // Add the encoded credentials to the server request
    strncat(serverRequest, credentials, SERVER_BUFFER_SIZE - 1);
    strncat(serverRequest, "\r\n", SERVER_BUFFER_SIZE - 1);

    if (debugNtripClientState)
    {
        Serial.print("NTRIP Client serverRequest size: ");
        Serial.print(strlen(serverRequest));
        Serial.print(" of ");
        Serial.print(sizeof(serverRequest));
        Serial.println(" bytes available");
        Serial.println("NTRIP Client sending server request: ");
        Serial.println(serverRequest);
    }

    // Send the server request
    ntripClient->write((const uint8_t *)serverRequest, strlen(serverRequest));
    ntripClientTimer = millis();
    return true;
}

//----------------------------------------
// Determine if another connection is possible or if the limit has been reached
bool ntripClientConnectLimitReached()
{
    int seconds;

    // Restart the NTRIP client
    ntripClientStop(true);

    // No more connection attempts, switching to Bluetooth
    Serial.println("NTRIP Client connection attempts exceeded!");
    return true;
}

//----------------------------------------
// Determine if NTRIP client data is available
int ntripClientReceiveDataAvailable()
{
    return ntripClient->available();
}

//----------------------------------------
// Read the response from the NTRIP client
void ntripClientResponse(char *response, size_t maxLength)
{
    char *responseEnd;

    // Make sure that we can zero terminate the response
    responseEnd = &response[maxLength - 1];

    // Read bytes from the caster and store them
    while ((response < responseEnd) && (ntripClientReceiveDataAvailable() > 0))
    {
        *response++ = ntripClient->read();
    }

    // Zero terminate the response
    *response = '\0';
}

//----------------------------------------
// Restart the NTRIP client
void ntripClientRestart()
{
    ntripClientConnectLimitReached();
}

//----------------------------------------
// Update the state of the NTRIP client state machine
void ntripClientSetState(uint8_t newState)
{
    if (debugNtripClientState)
    {
        if (ntripClientState == newState)
            Serial.print("NTRIP Client: *");
        else
            Serial.printf("NTRIP Client: %s --> ", ntripClientStateName[ntripClientState]);
    }
    ntripClientState = newState;
    if (debugNtripClientState)
    {
        if (newState >= NTRIP_CLIENT_STATE_MAX)
        {
            Serial.printf("Unknown client state: %d\r\n", newState);
            while (1);
        }
        else
            Serial.println(ntripClientStateName[ntripClientState]);
    }
}

//----------------------------------------
// Shutdown the NTRIP client
void ntripClientForceShutdown()
{
    ntripClientStop(true);
}

//----------------------------------------
// Start the NTRIP client
void ntripClientStart()
{
    // Start the NTRIP client
    Serial.println("NTRIP Client start");
    ntripClientStop(false);
}

//----------------------------------------
// Shutdown or restart the NTRIP client
void ntripClientStop(bool shutdown)
{
    if (ntripClient)
    {
        // Break the NTRIP client connection if necessary
        if (ntripClient->connected())
            ntripClient->stop();

        // Free the NTRIP client resources
        delete ntripClient;
        ntripClient = nullptr;
    }

    // Determine the next NTRIP client state
    if (shutdown)
    {
        ntripClientSetState(NTRIP_CLIENT_OFF);
    }
    else
        ntripClientSetState(NTRIP_CLIENT_NETWORK_STARTED);
}

//----------------------------------------
// Determine if NTRIP being used for correction data
bool ntripClientIsPushingRtcm()
{
    return (ntripClientState == NTRIP_CLIENT_CONNECTED);
}

//----------------------------------------
// Check for the arrival of any correction data. Push it to the GNSS.
// Stop task if the connection has dropped or if we receive no data for
// NTRIP_CLIENT_RECEIVE_DATA_TIMEOUT
void ntripClientUpdate()
{
    // Enable the network and the NTRIP client if requested
    switch (ntripClientState)
    {
    case NTRIP_CLIENT_OFF:
        break;

    // Wait for a network media connection
    case NTRIP_CLIENT_NETWORK_STARTED:
        // Determine if the network is connected to the media
        if (pppIsInternetAvailable())
        {
            // Allocate the ntripClient structure
            ntripClient = new NetworkClient();
            if (!ntripClient)
            {
                // Failed to allocate the ntripClient structure
                Serial.println("ERROR: Failed to allocate the ntripClient structure!");
                ntripClientForceShutdown();
            }
            else
            {
                // The network is available for the NTRIP client
                ntripClientSetState(NTRIP_CLIENT_NETWORK_CONNECTED);
            }
        }
        break;

    case NTRIP_CLIENT_NETWORK_CONNECTED:
        // Open connection to NTRIP caster service
        if (!ntripClientConnect())
        {
            // Assume service not available
            if (ntripClientConnectLimitReached())
                Serial.println(
                    "NTRIP caster failed to connect. Do you have your caster address and port correct?");
        }
        else
        {
            // Socket opened to NTRIP system
            if (debugNtripClientState)
                Serial.printf("NTRIP Client waiting for response from %s:%d\r\n",
                             casterHost, casterPort);
            ntripClientSetState(NTRIP_CLIENT_WAIT_RESPONSE);
        }
        break;

    case NTRIP_CLIENT_WAIT_RESPONSE:
        // Check for no response from the caster service
        if (ntripClientReceiveDataAvailable() <
                 strlen("ICY 200 OK")) // Wait until at least a few bytes have arrived
        {
            // Check for response timeout
            if (millis() - ntripClientTimer > NTRIP_CLIENT_RESPONSE_TIMEOUT)
            {
                // NTRIP web service did not respond
                if (ntripClientConnectLimitReached())
                    Serial.println("NTRIP Caster failed to respond. Do you have your caster address and port correct?");
            }
        }
        else
        {
            // Caster web service responded
            char response[512];
            ntripClientResponse(&response[0], sizeof(response));

            if (debugNtripClientState)
                Serial.printf("Caster Response: %s\r\n", response);
            else
                log_d("Caster Response: %s", response);

            // Look for various responses
            if (strstr(response, "200") != nullptr) //'200' found
            {
                // We got a response, now check it for possible errors
                if (strcasestr(response, "banned") != nullptr)
                {
                    Serial.printf("NTRIP Client connected to caster but caster responded with banned error: %s\r\n",
                                 response);

                    ntripClientConnectLimitReached(); // Re-attempted after a period of time. Shuts down NTRIP Client if
                                                      // limit reached.
                }
                else if (strcasestr(response, "sandbox") != nullptr)
                {
                    Serial.printf("NTRIP Client connected to caster but caster responded with sandbox error: %s\r\n",
                                 response);

                    ntripClientConnectLimitReached(); // Re-attempted after a period of time. Shuts down NTRIP Client if
                                                      // limit reached.
                }
                else if (strcasestr(response, "SOURCETABLE") != nullptr)
                {
                    Serial.printf("Caster may not have mountpoint %s. Caster responded with problem: %s\r\n",
                                 mountPoint, response);

                    // Stop NTRIP client operations
                    ntripClientForceShutdown();
                }
                else
                {
                    // We successfully connected
                    Serial.printf("NTRIP Client connected to %s:%d, receiving RTCM data\r\n", casterHost,
                                 casterPort);

                    // Connection is now open, start the NTRIP receive data timer
                    ntripClientTimer = millis();

                    // We don't use a task because we use I2C hardware (and don't have a semaphore).
                    ntripClientSetState(NTRIP_CLIENT_CONNECTED);
                }
            }
            else if (strstr(response, "401") != nullptr)
            {
                // Look for '401 Unauthorized'
                Serial.printf("NTRIP Caster responded with unauthorized error: %s. Are you sure your caster credentials "
                             "are correct?\r\n",
                             response);

                // Stop NTRIP client operations
                ntripClientForceShutdown();
            }
            // Other errors returned by the caster
            else
            {
                Serial.printf("NTRIP Client connected but caster responded with problem: %s\r\n", response);

                // Stop NTRIP client operations
                ntripClientForceShutdown();
            }
        }
        break;

    case NTRIP_CLIENT_CONNECTED:
        // Check for a broken connection
        if (!ntripClient->connected())
        {
            // Broken connection, retry the NTRIP client connection
            Serial.println("NTRIP Client connection to caster was broken");
            ntripClientRestart();
        }
        else
        {
            // Check for timeout receiving NTRIP data
            if (ntripClientReceiveDataAvailable() == 0)
            {
                // Don't fail during retransmission attempts
                if ((millis() - ntripClientTimer) > NTRIP_CLIENT_RECEIVE_DATA_TIMEOUT)
                {
                    // Timeout receiving NTRIP data, retry the NTRIP client connection
                    Serial.println("NTRIP Client timeout receiving data");
                    ntripClientRestart();
                }
            }
            else
            {
                // Receive data from the NTRIP Caster
                uint8_t rtcmData[RTCM_DATA_SIZE];
                size_t rtcmCount = 0;

                // Collect any available RTCM data
                if (ntripClientReceiveDataAvailable() > 0)
                {
                    rtcmCount = ntripClient->read(rtcmData, sizeof(rtcmData));
                    if (rtcmCount)
                    {
                        // Restart the NTRIP receive data timer
                        ntripClientTimer = millis();

                        // Push data to the GNSS
                        myGNSS.pushRawData(rtcmData, rtcmCount);
                        if (debugNtripClientRtcm)
                            Serial.printf(
                                "NTRIP Client received %d RTCM bytes, pushing to GNSS\r\n",
                                rtcmCount);
                    }
                }
            }
        }
        break;
    }
}
