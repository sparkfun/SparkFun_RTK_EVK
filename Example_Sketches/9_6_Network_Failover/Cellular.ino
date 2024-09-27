//----------------------------------------

// The following define is used to avoid confusion between the cellular
// code and the GNSS code where PPP has a completely different meaning.
// The remaining references to PPP in this code are present due to values
// from the PPP (Point-to-Point Protocol) driver and ARDUINO_EVENT_PPP_*
// events.
#define CELLULAR            PPP

//----------------------------------------
// Cellular interface definitions

#define CELLULAR_MODEM_MODEL    PPP_MODEM_GENERIC

#define CELLULAR_MODEM_APN      "internet"
#define CELLULAR_MODEM_PIN      NULL // Personal Identification Number: String in double quotes

#define CELLULAR_BOOT_DELAY     DELAY_SEC(15) // Delay after boot before starting cellular

#define CELLULAR_ATTACH_POLL_INTERVAL   100     // Milliseconds

//----------------------------------------
// Wait until the cellular modem is attached to a mobile network
void cellularAttached(uint8_t priority, uintptr_t parameter, bool debug)
{
    static uint32_t lastPollMsec;

    // Validate the priority
    networkPriorityValidation(priority);

    // Poll until the cellular modem attaches to a mobile network
    if ((millis() - lastPollMsec) >= CELLULAR_ATTACH_POLL_INTERVAL)
    {
        lastPollMsec = millis();
        if (CELLULAR.attached())
        {
            // Attached to a mobile network, continue 
            // Display the network information
            Serial.printf("Cellular attached to %s\r\n", CELLULAR.operatorName().c_str());
            if (debug)
            {
                Serial.print("    State: ");
                Serial.println(CELLULAR.radioState());
                Serial.print("    IMSI: ");
                Serial.println(CELLULAR.IMSI());
                Serial.print("    RSSI: ");
                Serial.println(CELLULAR.RSSI());
                int ber = CELLULAR.BER();
                if (ber > 0)
                {
                    Serial.print("    BER: ");
                    Serial.println(ber);
                }
            }

            // Switch into a state where data may be sent and received
            if (debug)
                Serial.println("Switching to data mode...");
            CELLULAR.mode(ESP_MODEM_MODE_CMUX);  // Data and Command mixed mode

            // Get the next sequence entry
            networkSequenceNextEntry(priority, debug);
        }
    }
}

//----------------------------------------
// Handle the cellular events
void cellularEvent(arduino_event_id_t event)
{
    IPAddress ipAddress;
    String manufacturer;
    String module;

    // Take the network offline if necessary
    if (networkIsInterfaceOnline(cellularPriority)
        && (event != ARDUINO_EVENT_ETH_GOT_IP)
        && (event != ARDUINO_EVENT_ETH_GOT_IP6)
        && (event != ARDUINO_EVENT_PPP_CONNECTED))
    {
        networkMarkOffline(cellularPriority);
    }

    // Handle the event
    switch (event)
    {
    default:
        Serial.printf("ERROR: Unknown ARDUINO_EVENT_PPP_* event, %d\r\n", event);
        break;

    case ARDUINO_EVENT_PPP_CONNECTED:
        Serial.println("Cellular Connected");
        networkMarkOnline(cellularPriority);
        break;

    case ARDUINO_EVENT_PPP_DISCONNECTED:
        Serial.println("Cellular Disconnected");
        break;

    case ARDUINO_EVENT_PPP_GOT_IP:
    case ARDUINO_EVENT_PPP_GOT_IP6:
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
        if (NETWORK_DEBUG_SEQUENCE)
            Serial.printf("    IMEI: %s\r\n", CELLULAR.IMEI().c_str());
        break;

    case ARDUINO_EVENT_PPP_STOP:
        Serial.println("Cellular Stopped");
        break;
    }
}

//----------------------------------------
void cellularStart(uint8_t priority, uintptr_t parameter, bool debug)
{
    // Validate the priority
    networkPriorityValidation(priority);

    // Configure the cellular modem
    CELLULAR.setApn(CELLULAR_MODEM_APN);
    CELLULAR.setPin(CELLULAR_MODEM_PIN);
    CELLULAR.setResetPin(CELLULAR_RST, CELLULAR_MODEM_RST_LOW); // v3.0.2 allows you to set the reset delay, but we don't need it
    CELLULAR.setPins(CELLULAR_TX, CELLULAR_RX, CELLULAR_RTS, CELLULAR_CTS, CELLULAR_MODEM_FC);

    // Now let the PPP turn the modem back on again if needed - with a 200ms reset
    // If the modem is on, this is too short to turn it off again
    Serial.println("Starting the modem. It might take a while!");

    // Starting the cellular modem
    CELLULAR.begin(CELLULAR_MODEM_MODEL);

    // Get the next sequence entry
    networkSequenceNextEntry(priority, debug);
}

//----------------------------------------
void cellularStop(uint8_t priority, uintptr_t parameter, bool debug)
{
    // Validate the priority
    networkPriorityValidation(priority);

    // Stopping the cellular modem
    Serial.println("Stopping the modem!");
    CELLULAR.mode(ESP_MODEM_MODE_COMMAND);
    CELLULAR.end();

    // Get the next sequence entry
    networkSequenceNextEntry(priority, debug);
}
