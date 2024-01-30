// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// processSocketData is provided to the cellular library via a 
// callback setter -- setSocketReadCallbackPlus. (See setup())
void processSocketData(int socket, const char *theData, int dataLength, IPAddress remoteAddress, int remotePort)
{
  Serial.print(F("processSocketData: Data received on socket "));
  Serial.print(socket);
  Serial.print(F(". Length is "));
  Serial.print(dataLength);
  
  if (connectionOpen)
  {
    Serial.println(F(". Pushing it to the GNSS..."));
    myGNSS.pushRawData((uint8_t *)theData, (size_t)dataLength);

    lastReceivedRTCM_ms = millis(); // Update lastReceivedRTCM_ms
  }
  else
  {
    Serial.println();
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// processSocketClose is provided to the cellular library via a 
// callback setter -- setSocketCloseCallback. (See setup())
void processSocketClose(int socket)
{
  Serial.print(F("processSocketClose: Socket "));
  Serial.print(socket);
  Serial.println(F(" closed!"));

  if (socket == socketNum)
  {
    socketNum = -1;
    connectionOpen = false;
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
