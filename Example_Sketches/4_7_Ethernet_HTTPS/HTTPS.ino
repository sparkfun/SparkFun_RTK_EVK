//----------------------------------------

enum HTTPS_STATE
{
    HTTPS_STATE_WAIT_NETWORK = 0,
    HTTPS_STATE_CONNECT_HOST,
    HTTPS_STATE_WAIT_RESPONSE,
    HTTPS_STATE_READ_PAGE,
    HTTPS_STATE_CLOSE_PAGE,
    HTTPS_STATE_INIT_DELAY,
    HTTPS_STATE_DELAY,
};

//----------------------------------------
// Read the remote host web-page
void httpsUpdate(HTTPS_CLIENT_CONNECTION * hcc, uint16_t portNumber, uint32_t delayMsec)
{
    int bytesRead;
    int dataBytes;
    const char * tagEnd = "</html>";
    const char * tagStart = "<html";

    switch (hcc->hccState)
    {
    // Wait for the PPP connection
    case HTTPS_STATE_WAIT_NETWORK:
        if (hcc->isInternetAvailable())
        {
            hcc->client.setCACert(hcc->rootCA);
            hcc->hccState = HTTPS_STATE_CONNECT_HOST;
        }
        break;

    // Connect to the remote host
    case HTTPS_STATE_CONNECT_HOST:
        // Has the network failed
        if (!hcc->isInternetAvailable())
        {
            Serial.println("Link failed");
            hcc->hccState = HTTPS_STATE_WAIT_NETWORK;
            break;
        }

        // Connect to the remote host
        if (!hcc->client.connect(hcc->hostName, portNumber))
        {
            Serial.printf("Connection to %s:%d failed!\r\n", hcc->hostName, portNumber);
            hcc->hccState = HTTPS_STATE_INIT_DELAY;
        }
        else
        {
            Serial.printf("Connection to %s:%d successful\r\n", hcc->hostName, portNumber);

            // Request the web page
            hcc->client.printf("GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", hcc->url, hcc->hostName);

            // No data read yet
            hcc->headerLength = 0;
            hcc->pageLength = 0;
            hcc->tagEndFound = false;
            hcc->tagEndOffset = 0;
            hcc->tagStartFound = false;
            hcc->tagStartOffset = 0;
            hcc->hccState = HTTPS_STATE_WAIT_RESPONSE;
        }
        break;

    case HTTPS_STATE_WAIT_RESPONSE:
        // Has the network failed or the connection closed
        if ((!hcc->isInternetAvailable()) || (!hcc->client.connected()))
        {
            Serial.println("Link failed or client disconnected");
            hcc->hccState = HTTPS_STATE_CLOSE_PAGE;
            break;
        }

        // Wait for the response
        if (hcc->client.available())
            hcc->hccState = HTTPS_STATE_READ_PAGE;
        break;

    // Read the web-page
    case HTTPS_STATE_READ_PAGE:
        // Has the network failed or the connection closed
        if ((!hcc->isInternetAvailable()) || (!hcc->client.connected()))
        {
            Serial.println("Link failed or client disconnected");
            hcc->hccState = HTTPS_STATE_CLOSE_PAGE;
            break;
        }

        // Check for end-of-file
        dataBytes = hcc->client.available();
        if (hcc->tagEndFound && (!dataBytes))
        {
            hcc->suppressFirstPageOutput = true;
            hcc->hccState = HTTPS_STATE_CLOSE_PAGE;
        }

        // Determine if data was received
        else if (dataBytes > 0)
        {
            // Read as much data as possible
            if (dataBytes > sizeof(hcc->buffer))
                dataBytes = sizeof(hcc->buffer);
            bytesRead = hcc->client.read(hcc->buffer, dataBytes);

            // Check for a read error
            if (bytesRead < 0)
            {
                Serial.println("\r\n\nRead error!");
                hcc->hccState = HTTPS_STATE_CLOSE_PAGE;
                break;
            }

            // Display the web page on the first pass
            if (!hcc->suppressFirstPageOutput)
                Serial.write(hcc->buffer, bytesRead);

            // Check for the start-of-page
            dataBytes = 0;
            if (!hcc->tagStartFound)
            {
                for (; dataBytes < bytesRead; dataBytes++)
                    if ((!hcc->tagStartFound) && (hcc->buffer[dataBytes] == tagStart[hcc->tagStartOffset]))
                    {
                        hcc->tagStartOffset += 1;
                        hcc->tagStartFound = (hcc->tagStartOffset == strlen(tagStart));
                        if (hcc->tagStartFound)
                        {
                            hcc->headerLength = hcc->pageLength
                                              + dataBytes
                                              - strlen(tagStart);
                            dataBytes += 1;
                            hcc->pageLength = - (dataBytes + strlen(tagStart));
                            break;
                        }
                    }
                    else
                        hcc->tagStartOffset = 0;
            }

            // Account for the data read
            hcc->pageLength += bytesRead;

            // Check for the end-of-page
            if (hcc->tagStartFound)
            {
                for (; dataBytes < bytesRead; dataBytes++)
                    if ((!hcc->tagEndFound) && (hcc->buffer[dataBytes] == tagEnd[hcc->tagEndOffset]))
                    {
                        hcc->tagEndOffset += 1;
                        hcc->tagEndFound = (hcc->tagEndOffset == strlen(tagEnd));
                    }
                    else
                        hcc->tagEndOffset = 0;
            }
        }
        break;

    // Close the socket
    case HTTPS_STATE_CLOSE_PAGE:
        // Done with this network client
        Serial.printf("\rRead %d header bytes and %d page bytes from %s\r\n",
                      hcc->headerLength, hcc->pageLength, hcc->hostName);
        hcc->client.stop();
        hcc->hccState = HTTPS_STATE_INIT_DELAY;
        break;

    // Start the delay
    case HTTPS_STATE_INIT_DELAY:
        Serial.println("----------");
        hcc->timer = millis();
        hcc->hccState = HTTPS_STATE_DELAY;
        break;

    // Delay for a while
    case HTTPS_STATE_DELAY:
        // Delay before attempting again
        if ((millis() - hcc->timer) >= delayMsec)
            hcc->hccState = HTTPS_STATE_WAIT_NETWORK;
        break;
    }
}
