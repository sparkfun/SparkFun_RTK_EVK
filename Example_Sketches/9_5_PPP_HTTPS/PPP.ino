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

            // Set LARA_PWR low
            // LARA_PWR is inverted by the RTK EVK level-shifter
            // High 'pushes' the LARA PWR_ON pin, toggling the power
            // Configure the pin here as PPP doesn't configure _pin_rst until .begin is called
            pinMode(LARA_PWR, OUTPUT);
            digitalWrite(LARA_PWR, LOW);

            // Now enable the 3.3V regulators for the GNSS and LARA
            pinMode(PWREN, OUTPUT);
            digitalWrite(PWREN, HIGH);

            delay(2000); // Wait for the power to stabilize

            // We don't know if the module is on or off
            // From the datasheet:
            //   Hold LARA_PWR pin low for 0.15 - 3.20s to switch module on
            //   Hold LARA_PWR pin low for 1.50s minimum to switch module off
            // (Remember that LARA_PWR is inverted by the RTK EVK level-shifter)
            // From initial power-on, the LARA will be off
            // If the LARA is already on, toggling LARA_PWR could turn it off...
            // If the LARA is already on and in a data state, an escape sequence (+++) (set_command_mode)
            // is needed to deactivate. PPP.begin tries this, but it fails on the LARA. Not sure why.
            // Also, the LARA takes ~8 seconds to start up after power on....

            // If the modem is off, a 2s push will turn it on
            // If the modem is on, a 2s push will turn it off
            Serial.println("Toggling the modem power");
            digitalWrite(LARA_PWR, HIGH);
            delay(2000);
            digitalWrite(LARA_PWR, LOW);
            delay(2000); // 1000 is too short

            // Now let the PPP turn the modem back on again if needed - with a 200ms reset
            // If the modem is on, this is too short to turn it off again
            Serial.println("Starting the modem. It might take a while!");
            PPP.setApn(PPP_MODEM_APN);
            PPP.setPin(PPP_MODEM_PIN);
            PPP.setResetPin(LARA_PWR, PPP_MODEM_RST_LOW); // v3.0.2 allows you to set the reset delay, but we don't need it
            PPP.setPins(LARA_TX, LARA_RX, LARA_RTS, LARA_CTS, PPP_MODEM_FC);
            pppState = PPP_STATE_LARA_ON; // Change pppState now. PPP_START / PPP_STOP will occur in the new state
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
        else if (event == ARDUINO_EVENT_PPP_STOP) // This happens when the PPP.begin fails
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
