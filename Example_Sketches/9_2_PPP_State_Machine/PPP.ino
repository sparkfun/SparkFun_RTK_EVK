//----------------------------------------

// Power control
#define PWREN             32

// LARA pins
#define LARA_RST          26    // Using the power pin as reset
#define LARA_TX           13
#define LARA_RX           14
#define LARA_RTS          -1
#define LARA_CTS          -1

// LARA R6001D no flow control
#define PPP_MODEM_RST_LOW false  //active HIGH
#define PPP_MODEM_FC      ESP_MODEM_FLOW_CONTROL_NONE
#define PPP_MODEM_MODEL   PPP_MODEM_GENERIC

#define PPP_MODEM_APN   "internet"
#define PPP_MODEM_PIN   NULL        // Personal Identification Number: String in double quotes

//----------------------------------------

// PPP states
enum PPP_STATES
{
    PPP_STATE_LARA_OFF = 0,     // LARA powered off or being reset
    PPP_STATE_LARA_ON,          // LARA powered on and out of reset
    PPP_STATE_MOBILE_NETWORK,   // Connect to the mobile network
    PPP_STATE_ATTACHED,         // Attached to a mobile network
    PPP_STATE_GOT_IP,           // Got an IP address
    PPP_STATE_CONNECTED,        // Connected to the internet
};

//----------------------------------------

static uint8_t pppState;
static uint32_t pppTimer;
bool pppOnline;

//----------------------------------------
// Determine if the PPP link is connected to the internet
bool pppIsInternetAvailable()
{
    return pppOnline;
}

//----------------------------------------
// Perform PPP polling
void pppUpdate()
{
    switch (pppState)
    {
    case PPP_STATE_MOBILE_NETWORK:
        if ((millis() - pppTimer) >= 100)
        {
            pppTimer = millis();
            if (PPP.attached())
            {
                Serial.println("Attached to moble network");
                Serial.print("    State: ");
                Serial.println(PPP.radioState());
                Serial.print("    Operator: ");
                Serial.println(PPP.operatorName());
                Serial.print("    IMSI: ");
                Serial.println(PPP.IMSI());
                Serial.print("    RSSI: ");
                Serial.println(PPP.RSSI());
                int ber = PPP.BER();
                if (ber > 0)
                {
                    Serial.print("    BER: ");
                    Serial.println(ber);
                }
                Serial.println("Switching to data mode...");
                pppOnline = false;
                pppState = PPP_STATE_ATTACHED;
                PPP.mode(ESP_MODEM_MODE_CMUX);  // Data and Command mixed mode
            }
        }
        break;
    }
}

//----------------------------------------
// Update the PPP state due to an event
void pppEvent(arduino_event_id_t event)
{
    switch (pppState)
    {
    default:
        Serial.printf("ERROR: Bad PPP state, %d\r\n", pppState);
        break;
    
    case PPP_STATE_LARA_OFF:
        if (event == ARDUINO_EVENT_LARA_ON)
        {
            // Configure the modem
            PPP.setApn(PPP_MODEM_APN);
            PPP.setPin(PPP_MODEM_PIN);
            PPP.setResetPin(LARA_RST, PPP_MODEM_RST_LOW);
            PPP.setPins(LARA_TX, LARA_RX, LARA_RTS, LARA_CTS, PPP_MODEM_FC);

            // Now enable the 3.3V regulators for the GNSS and LARA
            pinMode(PWREN, OUTPUT);
            digitalWrite(PWREN, HIGH);

            Serial.println("Starting the modem. It might take a while!");
            pppState = PPP_STATE_LARA_ON;
            pppOnline = false;
            PPP.begin(PPP_MODEM_MODEL);
        }
        break;

    case PPP_STATE_LARA_ON:
        if (event == ARDUINO_EVENT_PPP_START)
        {
            Serial.println("PPP Started");
            Serial.print("    Manufacturer: ");
            Serial.println(PPP.cmd("AT+CGMI", 10000));
            Serial.print("    Model: ");
            Serial.println(PPP.moduleName());
            Serial.print("    IMEI: ");
            Serial.println(PPP.IMEI());
            pppOnline = false;
            pppState = PPP_STATE_MOBILE_NETWORK;
        }
        else if (event == ARDUINO_EVENT_PPP_STOP)
        {
            Serial.println("PPP Stopped");
            pppOnline = false;
            pppState = PPP_STATE_LARA_OFF;
        }
        break;

    case PPP_STATE_ATTACHED:
        if (event == ARDUINO_EVENT_PPP_GOT_IP)
        {
            IPAddress ipv4 = PPP.localIP();
            Serial.print("PPP Got IP: ");
            Serial.print(ipv4);
            Serial.println();
            pppOnline = false;
            pppState = PPP_STATE_GOT_IP;
        }
        else if (event == ARDUINO_EVENT_PPP_DISCONNECTED)
        {
            Serial.println("PPP Disconnected");
            pppOnline = false;
            pppState = PPP_STATE_MOBILE_NETWORK;
        }
        else if (event == ARDUINO_EVENT_PPP_STOP)
        {
            Serial.println("PPP Stopped");
            pppOnline = false;
            pppState = PPP_STATE_LARA_OFF;
        }
        break;

    case PPP_STATE_GOT_IP:
        if (event == ARDUINO_EVENT_PPP_CONNECTED)
        {
            Serial.println("PPP Connected");
            pppOnline = true;
            pppState = PPP_STATE_CONNECTED;
        }
        else if (event == ARDUINO_EVENT_PPP_LOST_IP)
        {
            Serial.println("PPP Lost IP");
            pppOnline = false;
            pppState = PPP_STATE_ATTACHED;
        }
        else if (event == ARDUINO_EVENT_PPP_DISCONNECTED)
        {
            Serial.println("PPP Disconnected");
            pppOnline = false;
            pppState = PPP_STATE_MOBILE_NETWORK;
        }
        else if (event == ARDUINO_EVENT_PPP_STOP)
        {
            Serial.println("PPP Stopped");
            pppOnline = false;
            pppState = PPP_STATE_LARA_OFF;
        }
        break;

    case PPP_STATE_CONNECTED:
        if (event == ARDUINO_EVENT_PPP_LOST_IP)
        {
            Serial.println("PPP Lost IP");
            pppOnline = false;
            pppState = PPP_STATE_ATTACHED;
        }
        else if (event == ARDUINO_EVENT_PPP_DISCONNECTED)
        {
            Serial.println("PPP Disconnected");
            pppOnline = false;
            pppState = PPP_STATE_ATTACHED;
        }
        else if (event == ARDUINO_EVENT_PPP_STOP)
        {
            Serial.println("PPP Stopped");
            pppOnline = false;
            pppState = PPP_STATE_LARA_OFF;
        }
        break;
    }
}
