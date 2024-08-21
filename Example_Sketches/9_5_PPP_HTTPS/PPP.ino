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
    processEventQueue();

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
            // Note: the modem AT commands are defined in:
            // https://github.com/espressif/esp-protocols/blob/master/components/esp_modem/src/esp_modem_command_library.cpp

            PPP.setApn(PPP_MODEM_APN);
            PPP.setPin(PPP_MODEM_PIN);
            PPP.setPins(LARA_TX, LARA_RX, LARA_RTS, LARA_CTS, PPP_MODEM_FC);

            // Set LARA_PWR low. High 'pushes' the LARA PWR_ON pin, toggling the power.
            pinMode(LARA_PWR, OUTPUT);
            digitalWrite(LARA_PWR, LOW);

            // Now enable the 3.3V regulators for the GNSS and LARA
            pinMode(PWREN, OUTPUT);
            digitalWrite(PWREN, HIGH);

            delay(100); // Wait for the power to stabilize

            // From power-on, the LARA will be off. LARA_PWR needs to be toggled HIGH-LOW to turn it on.
            // BUT, if the LARA is already on, toggling LARA_PWR will turn it off.
            // If the LARA is already on and in a data state, an escape sequence (+++) may be needed to deactivate.
            // Also, the LARA takes 8 seconds to start up after power on....

            Serial.println("Starting the modem. It might take a while!");
            eventQueue.clear();
            if (PPP.begin(PPP_MODEM_MODEL))
            {
                pppState = PPP_STATE_LARA_ON;
                pppOnline = false;
            }
            else
            {
                PPP.end();
                PPP.setApn(PPP_MODEM_APN);
                PPP.setPin(PPP_MODEM_PIN);
                PPP.setPins(LARA_TX, LARA_RX, LARA_RTS, LARA_CTS, PPP_MODEM_FC);
                Serial.println("Toggling power and restarting the modem. It might take a while!");
                digitalWrite(LARA_PWR, HIGH);
                delay(3200); // Give LARA_PWR a long press - to turn it off or on
                digitalWrite(LARA_PWR, LOW);
                delay(8000);
                eventQueue.clear();
                if (PPP.begin(PPP_MODEM_MODEL)) // LARA _could_ now be off!
                {
                    pppState = PPP_STATE_LARA_ON;
                    pppOnline = false;
                }
                else
                {
                    PPP.end();
                    PPP.setApn(PPP_MODEM_APN);
                    PPP.setPin(PPP_MODEM_PIN);
                    PPP.setPins(LARA_TX, LARA_RX, LARA_RTS, LARA_CTS, PPP_MODEM_FC);
                    Serial.println("Toggling power and restarting the modem. It might take a while!");
                    digitalWrite(LARA_PWR, HIGH);
                    delay(100); // Give LARA_PWR a short press - to turn it on
                    digitalWrite(LARA_PWR, LOW);
                    delay(8000);
                    eventQueue.clear();
                    if (PPP.begin(PPP_MODEM_MODEL)) // Last chance...
                    {
                        pppState = PPP_STATE_LARA_ON;
                        pppOnline = false;
                    }
                    else
                    {
                        PPP.end();
                        Serial.println("LARA is not responding!");
                        // What to do? Stay in this state? Or go to PPP_STATE_LARA_ON anyway?
                    }
                }
            }
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
