/**********************************************************************
  WebServer.ino

  Web server for testing soft AP
**********************************************************************/

//*********************************************************************
// Respond to the HTTP request
void webServerUpdate(bool networkConnected)
{
    NetworkClient client;
    int lenRequest;
    int lenResponse;
    const char * response =
        // HTTP header
        "HTTP/1.1 200 OK\r\n"
        "Content-type:text/html\r\n"
        "\r\n"      // End of header

        // Web-page
        "<html>\r\n"
        "  <head>\r\n"
        "    <title>Soft AP</title>\r\n"
        "  </head>\r\n"
        "  <body>\r\n"
        "    <h1>Soft AP Response</h1>\r\n"
        "    <p>Connected to web server</p>\r\n"
        "  </body>\r\n"
        "</html>"
        "\r\n";     // End of web-page
    static NetworkServer * server;

    // Wait for the soft AP to be online
    if (wifi.softApOnline())
    {
        // Start the web server
        if (!server)
        {
            server = new NetworkServer(80);
            if (server)
                server->begin();
        }
        if (server)
        {
            client = server->accept();  // listen for incoming clients
            if (client)                 // if you get a client,
            {
                if (webServerDebug)
                    Serial.println("New Client.");  // print a message out the serial port
                String currentLine = "";        // make a String to hold incoming data from the client
                while (client.connected())    // loop while the client's connected
                {
                    lenRequest = 0;
                    while (client.available())       // if there's bytes to read from the client,
                    {
                        char c = client.read();     // read a byte, then
                        lenRequest += 1;
                    }

                    // Send the HTTP header and response
                    lenResponse = strlen(response);
                    client.print(response);
                    systemPrintf("\r*** Soft AP web server: Request length: %d, Response length: %d\r\n",
                                 lenRequest, lenResponse);

                    // Exit the loop
                    break;
                }

                // close the connection:
                client.stop();
                if (webServerDebug)
                    Serial.println("Client Disconnected.");
            }
        }
    }
    else
    {
        // Stop the web server
        if (server)
        {
            delete server;
            server = nullptr;
        }
    }
}
