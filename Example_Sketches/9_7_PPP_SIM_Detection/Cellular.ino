//----------------------------------------

// Power control
#define PWREN             32

// LARA pins
#define LARA_PWR          26
#define LARA_TX           13
#define LARA_RX           14
#define LARA_RTS          -1
#define LARA_CTS          -1

// LARA R6001D no flow control
#define CELLULAR_MODEM_RST_LOW false  //active HIGH
#define CELLULAR_MODEM_FC      ESP_MODEM_FLOW_CONTROL_NONE
#define CELLULAR_MODEM_MODEL   PPP_MODEM_GENERIC

#define CELLULAR_MODEM_APN   "internet"
#define CELLULAR_MODEM_PIN   NULL   // Personal Identification Number: String in double quotes

#define CELLULAR            PPP

//----------------------------------------

#define LARA_ON_TIME        (2 * 1000)  // Milliseconds
#define LARA_OFF_TIME       (5 * 1000)  // Milliseconds
#define LARA_SETTLE_TIME    (2 * 1000)  // Milliseconds

// LARA_PWR is inverted by the RTK EVK level-shifter
// High 'pushes' the LARA PWR_ON pin, toggling the power
#define LARA_PWR_LOW_VALUE  1
#define LARA_PWR_HIGH_VALUE 0

//----------------------------------------

static bool cellularAttached;
static bool cellularHasIP;
static String cellularSimCardID;
static bool cellularSimCardRemoved;
static bool cellularStarted;
static uint32_t cellularTimer;

//----------------------------------------
void laraOff()
{
    uint32_t pulseMsec;

    // Set LARA_PWR low
    digitalWrite(LARA_PWR, LARA_PWR_LOW_VALUE);

    // Configure the pin here as PPP doesn't configure _pin_rst until .begin is called
    pinMode(LARA_PWR, OUTPUT);

    // Generate the pulse to turn off the LARA
    pulseMsec = millis();
    while ((millis() - pulseMsec) < LARA_OFF_TIME);

    // Set the PWR pin high
    digitalWrite(LARA_PWR, LARA_PWR_HIGH_VALUE);
}

//----------------------------------------
void laraOn()
{
    uint32_t pulseMsec;

    // Set LARA_PWR low
    digitalWrite(LARA_PWR, LARA_PWR_LOW_VALUE);

    // Generate the pulse to turn on the LARA
    pulseMsec = millis();
    while ((millis() - pulseMsec) < LARA_ON_TIME);

    // Set the PWR pin high
    digitalWrite(LARA_PWR, LARA_PWR_HIGH_VALUE);
}

//----------------------------------------
// Reset the LARA chip
void laraReset()
{
    // Power cycle the LARA
    laraOff();
    laraOn();

    delay(500);
}

//----------------------------------------
// Determine if the cellular link is connected to the internet
bool cellularIsInternetAvailable()
{
    return CELLULAR.hasIP();
}

//----------------------------------------
// Perform cellular polling
void cellularUpdate(uint32_t currentMsec)
{
    if (cellularSimCardRemoved || (cellularStarted && cellularAttached))
        return;

    // Start the cellular modem
    if (!cellularStarted)
    {
        // Reset the LARA chip
        laraReset();

        // Configure the modem
        CELLULAR.setApn(CELLULAR_MODEM_APN);
        CELLULAR.setPin(CELLULAR_MODEM_PIN);
        CELLULAR.setResetPin(LARA_PWR, CELLULAR_MODEM_RST_LOW);
        CELLULAR.setPins(LARA_TX, LARA_RX, LARA_RTS, LARA_CTS, CELLULAR_MODEM_FC);

        Serial.println("Starting the modem. It might take a while!");
        CELLULAR.begin(CELLULAR_MODEM_MODEL);
        cellularStarted = true;
    }

    // Attach the cellular modem to the cellular network
    if (cellularStarted && (!cellularSimCardRemoved) && (!cellularAttached)
        && ((currentMsec - cellularTimer) >= 100))
    {
        cellularTimer = currentMsec;

        // Verify that the SIM card is installed
        if (cellularSimCardID.length() == 0)
        {
            Serial.printf("Checking for SIM card\r\n");
            cellularSimCardID = CELLULAR.cmd("AT+CCID", 500);
            if (cellularSimCardID.length())
                Serial.printf("SIM card installed\r\n");
            else
            {
                Serial.printf("SIM card removed\r\n");
                cellularSimCardRemoved = true;
            }
        }

        // Determine if the cellular modem is connected to the cellular network
        if ((!cellularSimCardRemoved) && CELLULAR.attached())
        {
            Serial.printf("Attached to moble network\r\n");
            Serial.printf("    State: %d\r\n", CELLULAR.radioState());
            Serial.printf("    Operator: %s\r\n", CELLULAR.operatorName().c_str());
            Serial.printf("    IMSI: %s\r\n", CELLULAR.IMSI().c_str());
            Serial.printf("    RSSI: %d", CELLULAR.RSSI());
            int ber = CELLULAR.BER();
            if (ber > 0)
            {
                Serial.printf("    BER: %d\r\n", ber);
            }

            // Connect the cellular modem to the internet
            Serial.printf("Switching to data mode...\r\n");
            CELLULAR.mode(ESP_MODEM_MODE_CMUX);  // Data and Command mixed mode
            cellularAttached = true;
        }
    }
}

//----------------------------------------
// Handle the cellular events
void cellularEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    IPAddress ipAddress;
    String manufacturer;
    String module;

    // Determine if cellular lost its IP address
    if ((event != ARDUINO_EVENT_PPP_GOT_IP) && (event != ARDUINO_EVENT_PPP_GOT_IP6))
        cellularHasIP = false;

    // Cellular State Machine
    //
    //   .--------+<----------+<-----------+<---------.
    //   v        |           |            |          |
    // STOP --> START --> ATTACHED --> GOT_IP --> CONNECTED --> LOST_IP --> DISCONNECTED
    //   ^                    ^            ^          |            |            |
    //   |                    |            '----------+<-----------'            |
    //   '--------------------+<------------------------------------------------'
    //
    // Handle the event
    switch (event)
    {
    default:
        Serial.printf("ERROR: Unknown ARDUINO_EVENT_PPP_* event, %d\r\n", event);
        break;

    case ARDUINO_EVENT_PPP_CONNECTED:
        Serial.println("Cellular Connected");
        break;

    case ARDUINO_EVENT_PPP_DISCONNECTED:
        Serial.println("Cellular Disconnected");
        cellularAttached = false;
        break;

    case ARDUINO_EVENT_PPP_GOT_IP:
    case ARDUINO_EVENT_PPP_GOT_IP6:
        cellularHasIP = true;
        ipAddress = CELLULAR.localIP();
        Serial.printf("Cellular IP address: %s\r\n", ipAddress.toString().c_str());
        break;

    case ARDUINO_EVENT_PPP_LOST_IP:
        Serial.println("Cellular Lost IP address");
        break;

    case ARDUINO_EVENT_PPP_START:
        manufacturer = CELLULAR.cmd("AT+CGMI", 10000);
        module = CELLULAR.moduleName();
        Serial.printf("Cellular (%s %s) Started\r\n", manufacturer.c_str(), module.c_str());
        Serial.printf("    IMEI: %s\r\n", CELLULAR.IMEI().c_str());
        break;

    case ARDUINO_EVENT_PPP_STOP:
        Serial.println("Cellular Stopped");
        cellularStarted = false;
        cellularAttached = false;
        break;
    }
}
