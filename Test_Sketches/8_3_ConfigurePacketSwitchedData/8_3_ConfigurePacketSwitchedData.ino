/*
  SparkFun RTK Control Test Sketch

  License: MIT. Please see LICENSE.md for more details

  ESP32-WROVER-IE Pin Allocations:
  D0  : Boot + Boot Button
  D1  : Serial TX (CH340 RX)
  D2  : STAT LED
  D3  : Serial RX (CH340 TX)
  D4  : SD CS
  D5  : Unused - via 74HC4066 switch
  D12 : SDA2 - Qwiic OLED - via 74HC4066 switch
  D13 : Serial1 TX - LARA_TXDI
  D14 : Serial1 RX - LARA RXDO
  D15 : SCL2 - Qwiic OLED - via 74HC4066 switch
  D16 : N/C
  D17 : N/C
  D18 : SPI SCK
  D19 : SPI POCI
  D21 : I2C SDA
  D22 : I2C SCL
  D23 : SPI PICO
  D25 : GNSS Time Pulse
  D26 : LARA Power On
  D27 : Ethernet Chip Select
  D32 : PWREN
  D33 : Ethernet Interrupt
  A34 : LARA Network Indicator
  A35 : Board Detect (3.0V)
  A36 : SD Card Detect
*/

const int SD_CS = 4; // Chip select for the microSD card
const int ETHERNET_CS = 27; // Chip select for the WizNet 5500
const int PWREN = 32; // 3V3_SW and SDIO Enable
const int STAT_LED = 2;
const int SCL_2 = 15;
const int SDA_2 = 12;
const int SERIAL_TX = 13;
const int SERIAL_RX = 14;
const int LARA_PWR = 26;
const int LARA_NI = 34;

#include <HardwareSerial.h>
HardwareSerial laraSerial(2); //UART2 normally uses pins 16 and 17, but these are not available on WROVER

#include <SparkFun_u-blox_SARA-R5_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_u-blox_SARA-R5_Arduino_Library

// Create a LARA_R6 object to use throughout the sketch
SARA_R5 myLARA;

// Create a SARA_R5 object to use throughout the sketch
// We can also tell the library what GPIO pin is connected to the SARA power pin.
//SARA_R5 myLARA(LARA_PWR);

// processPSDAction is provided to the SARA-R5 library via a 
// callback setter -- setPSDActionCallback. (See setup())
void processPSDAction(int result, IPAddress ip)
{
  Serial.println();
  Serial.print(F("PSD Action:  result: "));
  Serial.print(String(result));
  if (result == 0)
    Serial.print(F(" (success)"));
  Serial.print(F("  IP Address: \""));
  Serial.print(String(ip[0]));
  Serial.print(F("."));
  Serial.print(String(ip[1]));
  Serial.print(F("."));
  Serial.print(String(ip[2]));
  Serial.print(F("."));
  Serial.print(String(ip[3]));
  Serial.println(F("\""));
}

void setup()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(ETHERNET_CS, OUTPUT);
  digitalWrite(ETHERNET_CS, HIGH);
  pinMode(PWREN, OUTPUT);
  digitalWrite(PWREN, HIGH);
  pinMode(LARA_NI, INPUT);
  
  pinMode(LARA_PWR, OUTPUT); //LARA_PWR is not inverted. But probably should be...
  digitalWrite(LARA_PWR, HIGH);

  laraSerial.begin(115200, SERIAL_8N1, SERIAL_RX, SERIAL_TX); // Configure Serial1
  
  String currentOperator = "";

  delay(1000);

  Serial.begin(115200);
  Serial.println("SparkFun RTK - Test Sketch");

  //Wait for the LARA_NI pin to go high
  Serial.print("Waiting for the LARA NI pin to go high");
  do {
    Serial.print(".");
    delay(1000);
  } while (digitalRead(LARA_NI) == LOW);
  Serial.println();
  
  //myLARA.enableDebugging();

  myLARA.invertPowerPin(false); //LARA_PWR is not inverted. But probably should be...
  
  // Initialize the LARA
  if (myLARA.begin(laraSerial, 115200) )
  {
    Serial.println(F("LARA-R6 connected!"));
  }
  else
  {
    Serial.println(F("Unable to communicate with the LARA."));
  }

  while (Serial.available()) Serial.read();
  
  // First check to see if we're connected to an operator:
  if (myLARA.getOperator(&currentOperator) == SARA_R5_SUCCESS)
  {
    Serial.print(F("Connected to: "));
    Serial.println(currentOperator);
  }
  else
  {
    Serial.print(F("The LARA is not yet connected to an operator. Please use the previous examples to connect. Or wait and retry. Freezing..."));
    while (1)
      ; // Do nothing more
  }

  int minCID = SARA_R5_NUM_PDP_CONTEXT_IDENTIFIERS; // Keep a record of the highest and lowest CIDs
  int maxCID = 0;

  Serial.println(F("The available Context IDs are:"));
  Serial.println(F("Context ID:\tAPN Name:\tIP Address:"));
  for (int cid = 0; cid < SARA_R5_NUM_PDP_CONTEXT_IDENTIFIERS; cid++)
  {
    String apn = "";
    IPAddress ip(0, 0, 0, 0);
    myLARA.getAPN(cid, &apn, &ip);
    if (apn.length() > 0)
    {
      Serial.print(cid);
      Serial.print(F("\t"));
      Serial.print(apn);
      Serial.print(F("\t"));
      Serial.println(ip);
    }
    if (cid < minCID)
      minCID = cid; // Record the lowest CID
    if (cid > maxCID)
      maxCID = cid; // Record the highest CID
  }
  Serial.println();

  Serial.println(F("Which Context ID do you want to use for your Packet Switched Data connection?"));
  Serial.println(F("Please enter the number (followed by LF / Newline): "));
  
  char c = 0;
  bool selected = false;
  int selection = 0;
  while (!selected)
  {
    while (!Serial.available()) ; // Wait for a character to arrive
    c = Serial.read(); // Read it
    if (c == '\n') // Is it a LF?
    {
      if ((selection >= minCID) && (selection <= maxCID))
      {
        selected = true;
        Serial.println("Using CID: " + String(selection));
      }
      else
      {
        Serial.println(F("Invalid CID. Please try again:"));
        selection = 0;
      }
    }
    else
    {
      selection *= 10; // Multiply selection by 10
      selection += c - '0'; // Add the new digit to selection      
    }
  }

  // Deactivate the profile
  if (myLARA.performPDPaction(0, SARA_R5_PSD_ACTION_DEACTIVATE) != SARA_R5_SUCCESS)
  {
    Serial.println(F("Warning: performPDPaction (deactivate profile) failed. Probably because no profile was active."));
  }

  // Map PSD profile 0 to the selected CID
  if (myLARA.setPDPconfiguration(0, SARA_R5_PSD_CONFIG_PARAM_MAP_TO_CID, selection) != SARA_R5_SUCCESS)
  {
    Serial.println(F("setPDPconfiguration (map to CID) failed! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Set the protocol type - this needs to match the defined IP type for the CID (as opposed to what was granted by the network)
  if (myLARA.setPDPconfiguration(0, SARA_R5_PSD_CONFIG_PARAM_PROTOCOL, SARA_R5_PSD_PROTOCOL_IPV4V6_V4_PREF) != SARA_R5_SUCCESS)
  // You _may_ need to change the protocol type: ----------------------------------------^
  {
    Serial.println(F("setPDPconfiguration (set protocol type) failed! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Set a callback to process the results of the PSD Action
  myLARA.setPSDActionCallback(&processPSDAction);

  // Activate the profile
  if (myLARA.performPDPaction(0, SARA_R5_PSD_ACTION_ACTIVATE) != SARA_R5_SUCCESS)
  {
    Serial.println(F("performPDPaction (activate profile) failed! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  for (int i = 0; i < 100; i++) // Wait for up to a second for the PSD Action URC to arrive
  {
    myLARA.poll(); // Keep processing data from the SARA so we can process the PSD Action
    delay(10);
  }

  // Save the profile to NVM - so we can load it again in the later examples
  if (myLARA.performPDPaction(0, SARA_R5_PSD_ACTION_STORE) != SARA_R5_SUCCESS)
  {
    Serial.println(F("performPDPaction (save to NVM) failed! Freezing..."));
    while (1)
      ; // Do nothing more
  }

}

void loop()
{
  myLARA.poll(); // Keep processing data from the SARA so we can process URCs from the PSD Action
}
