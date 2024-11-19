/*
 WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
*/

#define LED_STATUS          2

NetworkClient client;
String headerLine;
NetworkServer * server;

// Turn off the LED
void ledOff()
{
    digitalWrite(LED_STATUS, LOW);
}

// Turn on the LED
void ledOn()
{
    digitalWrite(LED_STATUS, HIGH);
}

// Initialize the server
bool serverBegin(IPAddress ipAddress, uint16_t port)
{
    // Turn off the LED
    ledOff();
    pinMode(LED_STATUS, OUTPUT);

    // Allocate the server structure
    server = new NetworkServer(ipAddress, port);
    if (server)
    {
        server->begin();
    
        // Display the server IP address and port for remote connections
        Serial.printf("WiFi Server: %s:%d\r\n", ipAddress.toString().c_str(), port);
    }
    else
        Serial.printf("ERROR: Failed to allocate the server!\r\n");

    // Return the initialization status
    return (server != nullptr);
}

void serverProcessRequest()
{
    int getPosition;
    
    // Read the HTTP request from the remote client
    while (client.available())
    {
        char c = client.read();

        // Ignore this line
        if (c == '\n')
            headerLine = "";

        // Build the header line
        else if (c != '\r')
            headerLine += c;

        // The HTTP header ends with a blank line
        // Display the web-page after receiving the entire request
        else if (headerLine.length() == 0)
        {
            // Respond with the web page contents
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // Web page contents
            client.printf("<html>\r\n");
            client.printf("  <head>\r\n");
            client.printf("    <title>LED Control</title>\r\n");
            client.printf("  </head>\r\n");
            client.printf("  <body>\r\n");
            client.printf("    <h1>Status LED Control</h1>\r\n");
            client.printf("    Click <a href=\"/H\">here</a> to turn the status LED (pin %d) on.<br>\r\n", LED_STATUS);
            client.printf("    Click <a href=\"/L\">here</a> to turn the status LED (pin %d) off.<br>\r\n", LED_STATUS);
            client.printf("  </body>\r\n");
            client.printf("</html>\r\n");

            // The HTTP response ends with another blank line:
            client.println();

            // Break the client connection
            client.stop();
        }

        // Process the request
        if (headerLine.endsWith("GET /H"))
            ledOn();
        else if (headerLine.endsWith("GET /L"))
            ledOff();
    }
}

// Update the server
void serverUpdate()
{
    // Wait for a client connection
    if (!client)
    {
        client = server->accept();
        if (client)
            headerLine = "";
    }

    // Determine is a new client connection request was received
    if (client)
    {
        // Process the request from the server
        if (client.connected())
            serverProcessRequest();
        else
        {
            client.stop();
        }
    }
}
