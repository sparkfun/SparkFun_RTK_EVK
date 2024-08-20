#include <PPP.h>

#define PPP_MODEM_APN   "internet"
#define PPP_MODEM_PIN   NULL        // Personal Identification Number: String in double quotes

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

void onEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    IPAddress ipv4;

    switch (event)
    {
    case ARDUINO_EVENT_PPP_START:        Serial.println("PPP Started"); break;
    case ARDUINO_EVENT_PPP_CONNECTED:    Serial.println("PPP Connected"); break;
    case ARDUINO_EVENT_PPP_GOT_IP:
        ipv4 = PPP.localIP();
        Serial.print("PPP Got IP: ");
        Serial.print(ipv4);
        Serial.println();
        break;
    case ARDUINO_EVENT_PPP_LOST_IP:      Serial.println("PPP Lost IP"); break;
    case ARDUINO_EVENT_PPP_DISCONNECTED: Serial.println("PPP Disconnected"); break;
    case ARDUINO_EVENT_PPP_STOP:         Serial.println("PPP Stopped"); break;
    default:                             break;
    }
}

void testClient(const char *host, uint16_t port) {
    NetworkClient client;
    static bool suppressOutput = false;
    if (!client.connect(host, port))
    {
        Serial.printf("Connection to %s:%d failed!\r\n", host, port);
        return;
    }
    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    while (client.connected() && !client.available());
    Serial.printf("Connection to %s:%d successful\r\n", host, port);
    int bytesRead = 0;
    while (client.available())
    {
        client.read();
        if (!suppressOutput)
            Serial.write(client.read());
        bytesRead += 1;
    }
    if (!suppressOutput)
        Serial.println();
    suppressOutput = true;

    Serial.printf("Read %d bytes from %s\r\n", bytesRead, host);
    client.stop();
}

void setup() {
    Serial.begin(115200);

    // Listen for modem events
    Network.onEvent(onEvent);

    // Configure the modem
    PPP.setApn(PPP_MODEM_APN);
    PPP.setPin(PPP_MODEM_PIN);
    PPP.setResetPin(LARA_RST, PPP_MODEM_RST_LOW);
    PPP.setPins(LARA_TX, LARA_RX, LARA_RTS, LARA_CTS, PPP_MODEM_FC);

    // Now enable the 3.3V regulators for the GNSS and LARA
    pinMode(PWREN, OUTPUT);
    digitalWrite(PWREN, HIGH);

    Serial.println("Starting the modem. It might take a while!");
    PPP.begin(PPP_MODEM_MODEL);

    Serial.print("Manufacturer: ");
    Serial.println(PPP.cmd("AT+CGMI", 10000));
    Serial.print("Model: ");
    Serial.println(PPP.moduleName());
    Serial.print("IMEI: ");
    Serial.println(PPP.IMEI());

    bool attached = PPP.attached();
    if (!attached)
    {
        int i = 0;
        unsigned int s = millis();
        Serial.print("Waiting to connect to network");
        while (!attached && ((++i) < 600))
        {
            Serial.print(".");
            delay(100);
            attached = PPP.attached();
        }
        Serial.print((millis() - s) / 1000.0, 1);
        Serial.println("s");
        attached = PPP.attached();
    }

    Serial.print("Attached: ");
    Serial.println(attached);
    Serial.print("State: ");
    Serial.println(PPP.radioState());
    if (attached)
    {
        Serial.print("Operator: ");
        Serial.println(PPP.operatorName());
        Serial.print("IMSI: ");
        Serial.println(PPP.IMSI());
        Serial.print("RSSI: ");
        Serial.println(PPP.RSSI());
        int ber = PPP.BER();
        if (ber > 0)
        {
            Serial.print("BER: ");
            Serial.println(ber);
            Serial.print("NetMode: ");
            Serial.println(PPP.networkMode());
        }

        Serial.println("Switching to data mode...");
        PPP.mode(ESP_MODEM_MODE_CMUX);  // Data and Command mixed mode
        if (!PPP.waitStatusBits(ESP_NETIF_CONNECTED_BIT, 1000))
        {
            Serial.println("Failed to connect to internet!");
        }
        else
        {
            Serial.println("Connected to internet!");
        }
    }
    else
    {
        Serial.println("Failed to connect to network!");
    }
}

void loop() {
    if (PPP.connected())
    {
        testClient("google.com", 80);
    }
    delay(20000);
}
