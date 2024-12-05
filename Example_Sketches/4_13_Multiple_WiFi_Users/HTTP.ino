/**********************************************************************
  HTTP.ino

  Connect to a remote web-site
**********************************************************************/

//****************************************
// Constants
//****************************************

enum HTTP_STATE
{
    HTTP_STATE_WAIT_NETWORK = 0,
    HTTP_STATE_CONNECT_HOST,
    HTTP_STATE_WAIT_RESPONSE,
    HTTP_STATE_READ_PAGE,
    HTTP_STATE_CLOSE_PAGE,
    HTTP_STATE_NO_NETWORK,
};

const char * tagEnd = "</html>";
const char * tagStart = "<html";

//*********************************************************************
// Constructor
HTTP_CLIENT_CONNECTION::HTTP_CLIENT_CONNECTION(const char * hostName,
                                               uint16_t portNumber,
                                               const char * url)
    : _hostName{hostName}, _url{url}, _client{nullptr}, _headerLength{0},
      _pageLength{0}, _portNumber{portNumber}, _hccState{0}, _tagEndFound{false},
      _tagEndOffset{0}, _tagStartFound{false}, _tagStartOffset{0},
      _timer{0}, _buffer{{0}}, _suppressFirstPageOutput{false}
{
}

//*********************************************************************
HTTP_CLIENT_CONNECTION::~HTTP_CLIENT_CONNECTION()
{
}

//*********************************************************************
// Read the HTTP web-page
void HTTP_CLIENT_CONNECTION::update(bool networkConnected)
{
    int bytesRead;
    int dataBytes;

    switch (_hccState)
    {
    // Wait for the PPP connection
    case HTTP_STATE_WAIT_NETWORK:
        if (networkConnected)
            _hccState = HTTP_STATE_CONNECT_HOST;
        break;

    // Connect to Google
    case HTTP_STATE_CONNECT_HOST:
        // Has the network failed
        if (!networkConnected)
        {
            _hccState = HTTP_STATE_WAIT_NETWORK;
            break;
        }

        // Allocate the client
        _client = new NetworkClient();
        if (!_client)
        {
            Serial.printf("Failed to allocate network client!\r\n");
            _hccState = HTTP_STATE_NO_NETWORK;
        }

        // Connect to the remote host
        if (!_client->connect(_hostName, _portNumber))
        {
            Serial.printf("Connection to %s:%d failed!\r\n", _hostName, _portNumber);
            _hccState = HTTP_STATE_NO_NETWORK;
        }
        else
        {
            Serial.printf("Connection to %s:%d successful\r\n", _hostName, _portNumber);

            // Request the web page
            _client->printf("GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", _url, _hostName);

            // No data read yet
            _headerLength = 0;
            _pageLength = 0;
            _tagEndFound = false;
            _tagEndOffset = 0;
            _tagStartFound = false;
            _tagStartOffset = 0;
            _hccState = HTTP_STATE_WAIT_RESPONSE;
        }
        break;

    case HTTP_STATE_WAIT_RESPONSE:
        // Has the network failed or the connection closed
        if ((!networkConnected) || (!_client->connected()))
        {
            _hccState = HTTP_STATE_CLOSE_PAGE;
            break;
        }

        // Wait for the response
        if (_client->available())
            _hccState = HTTP_STATE_READ_PAGE;
        break;

    // Read the web-page
    case HTTP_STATE_READ_PAGE:
        // Has the network failed or the connection closed
        if ((!networkConnected) || (!_client->connected()))
        {
            _hccState = HTTP_STATE_CLOSE_PAGE;
            break;
        }

        // Check for end-of-file
        dataBytes = _client->available();
        if (_tagEndFound && (!dataBytes))
        {
            _suppressFirstPageOutput = true;
            _hccState = HTTP_STATE_CLOSE_PAGE;
        }

        // Determine if data was received
        else if (dataBytes > 0)
        {
            // Read as much data as possible
            if (dataBytes > sizeof(_buffer))
                dataBytes = sizeof(_buffer);
            bytesRead = _client->read(_buffer, dataBytes);

            // Check for a read error
            if (bytesRead < 0)
            {
                Serial.println("\r\n\nRead error!");
                _hccState = HTTP_STATE_CLOSE_PAGE;
                break;
            }

            // Display the web page on the first pass
            if (!_suppressFirstPageOutput)
                Serial.write(_buffer, bytesRead);

            // Check for the start-of-page
            dataBytes = 0;
            if (!_tagStartFound)
            {
                for (; dataBytes < bytesRead; dataBytes++)
                    if ((!_tagStartFound) && (_buffer[dataBytes] == tagStart[_tagStartOffset]))
                    {
                        _tagStartOffset += 1;
                        _tagStartFound = (_tagStartOffset == strlen(tagStart));
                        if (_tagStartFound)
                        {
                            _headerLength = _pageLength
                                               + dataBytes
                                               - strlen(tagStart);
                            dataBytes += 1;
                            _pageLength = - (dataBytes + strlen(tagStart));
                            break;
                        }
                    }
                    else
                        _tagStartOffset = 0;
            }

            // Account for the data read
            _pageLength += bytesRead;

            // Check for the end-of-page
            if (_tagStartFound)
            {
                for (; dataBytes < bytesRead; dataBytes++)
                    if ((!_tagEndFound) && (_buffer[dataBytes] == tagEnd[_tagEndOffset]))
                    {
                        _tagEndOffset += 1;
                        _tagEndFound = (_tagEndOffset == strlen(tagEnd));
                    }
                    else
                        _tagEndOffset = 0;
            }
        }
        break;

    // Close the socket
    case HTTP_STATE_CLOSE_PAGE:
        // Done with this network client
        Serial.printf("\r*** WiFi STA: Read %d header bytes and %d page bytes from %s\r\n",
                      _headerLength, _pageLength, _hostName);
        _client->stop();
        delete (_client);
        _client = nullptr;
        _hccState = HTTP_STATE_NO_NETWORK;
        break;

    // Start the delay
    case HTTP_STATE_NO_NETWORK:
        if (!networkConnected)
            _hccState = HTTP_STATE_WAIT_NETWORK;
        break;
    }
}

//*********************************************************************
// Read the HTTP web-page
void httpUpdate(HTTP_CLIENT_CONNECTION * hcc, bool networkConnected)
{
    hcc->update(networkConnected);
}
