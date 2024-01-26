/*
  SparkFun RTK EVK Test Sketch

  License: MIT. Please see LICENSE.md for more details

  This example demonstrates how to connect the LARA_R6 to a new operator.

  Important note:
  Please select a LTE or UTRAN operator only.
  Selecting GSM will cause high peak currents during transmit and could cause the power to brown-out.

  ESP32-WROVER-IE Pin Allocations:
  D0  : Boot + Boot Button
  D1  : Serial TX (CH340 RX)
  D2  : STAT LED
  D3  : Serial RX (CH340 TX)
  D4  : SD CS
  D5  : LARA_ON - via 74HC4066 switch and PWREN. Needs PULLDOWN
  D12 : SDA2 - Qwiic OLED - via 74HC4066 switch and PWREN
  D13 : Serial1 TX - LARA_TXDI
  D14 : Serial1 RX - LARA RXDO
  D15 : SCL2 - Qwiic OLED - via 74HC4066 switch and PWREN
  D16 : N/A
  D17 : N/A
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

const int STAT_LED = 2;
const int SD_CS = 4; // Chip select for the microSD card
const int LARA_ON = 5; // High indicates the LARA VCCIO is on. Needs PULLDOWN
const int SDA_2 = 12; // OLED
const int SERIAL_TX = 13; // LARA_TXDI
const int SERIAL_RX = 14; // LARA RXDO
const int SCL_2 = 15; // OLED
const int SCL_1 = 22; // ZED-F9P and NEO-D9S
const int SDA_1 = 21; // ZED-F9P and NEO-D9S
const int GNSS_INT = 25; // ZED_F9P interrupt
const int LARA_PWR = 26; // LARA_PWR_ON - inverted - set LARA_PWR high to pull LARA_PWR_ON low
const int ETHERNET_CS = 27; // Chip select for the WizNet W5500
const int PWREN = 32; // 74HC4066 switch Enable - pull high to enable SCL2/SDA2 and LARA_ON
const int ETHERNET_INT = 33; // WizNet W5500 interrupt
const int LARA_NI = 34; // LARA Network Indicator - only valid when the LARA is powered on
const int SD_PRESENT = 36; // microSD card card present - from the microSD socket switch

#include <HardwareSerial.h>
HardwareSerial laraSerial(2); //UART2 normally uses pins 16 and 17, but these are not available on WROVER

#include <SparkFun_u-blox_SARA-R5_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_u-blox_SARA-R5_Arduino_Library

// Derive the SARA_R5 class, so we can override beginSerial
class SARA_R5_DERIVED : public SARA_R5
{
  public:
    SARA_R5_DERIVED() : SARA_R5{LARA_PWR}{} // Pass the LARA_PWR pin into the class so the library can powerOn / powerOff

  protected:
    void beginSerial(unsigned long baud) override
    {
      delay(100);
      if (_hardSerial != nullptr)
      {
        _hardSerial->end();
        _hardSerial->begin(baud, SERIAL_8N1, SERIAL_RX, SERIAL_TX); // Configure Serial1
      }
      delay(100);
    }
};

// Create a LARA_R6 object to use throughout the sketch
SARA_R5_DERIVED myLARA;

// Map registration status messages to more readable strings
String registrationString[] =
{
  "Not registered",                         // 0
  "Registered, home",                       // 1
  "Searching for operator",                 // 2
  "Registration denied",                    // 3
  "Registration unknown",                   // 4
  "Registered, roaming",                    // 5
  "Registered, home (SMS only)",            // 6
  "Registered, roaming (SMS only)",         // 7
  "Registered, emergency service only",     // 8
  "Registered, home, CSFB not preferred",   // 9
  "Registered, roaming, CSFB not prefered"  // 10
};

// Network operator can be set to e.g.:
// MNO_SW_DEFAULT -- DEFAULT (Undefined / regulatory)
// MNO_SIM_ICCID -- SIM DEFAULT
// MNO_ATT -- AT&T 
// MNO_VERIZON -- Verizon
// MNO_TELSTRA -- Telstra
// MNO_TMO -- T-Mobile US
// MNO_CT -- China Telecom
// MNO_SPRINT
// MNO_VODAFONE
// MNO_NTT_DOCOMO
// MNO_TELUS
// MNO_SOFTBANK
// MNO_DT -- Deutsche Telekom
// MNO_US_CELLULAR
// MNO_SKT
// MNO_GLOBAL -- SARA factory-programmed value
// MNO_STD_EUROPE
// MNO_STD_EU_NOEPCO

// MNO_GLOBAL is the factory-programmed default
// If you are in Europe, you may find no operators unless you choose MNO_STD_EUROPE
const mobile_network_operator_t MOBILE_NETWORK_OPERATOR = MNO_GLOBAL;

const String MOBILE_NETWORK_STRINGS[] = {"default (Undefined/Regulatory)", "SIM ICCID", "AT&T", "Verizon", 
  "Telstra", "T-Mobile US", "China Telecom", "Sprint", "Vodafone", "NTT DoCoMo", "Telus", "SoftBank",
  "Deutsche Telekom", "US Cellular", "SKT", "global (factory default)", "standard Europe",
  "standard Europe No-ePCO", "NOT RECOGNIZED"};

// Convert the operator number into an index for MOBILE_NETWORK_STRINGS
int convertOperatorNumber( mobile_network_operator_t mno)
{
  switch (mno)
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return ((int)mno);
      break;
    case 8:
      return 7;
      break;
    case 19:
      return 8;
      break;
    case 20:
      return 9;
      break;
    case 21:
      return 10;
      break;
    case 28:
      return 11;
      break;
    case 31:
      return 12;
      break;
    case 32:
      return 13;
      break;
    case 39:
      return 14;
      break;
    case 90:
      return 15;
      break;
    case 100:
      return 16;
      break;
    case 101:
      return 17;
      break;
    default: // NOT RECOGNIZED
      return 18;
      break;
  }
}

// This defines the size of the ops struct array. To narrow the operator
// list, set MOBILE_NETWORK_OPERATOR to AT&T, Verizon etc. instead
// of MNO_SW_DEFAULT.
#define MAX_OPERATORS 20

// Uncomment this line if you want to be able to communicate directly with the SARA in the main loop
//#define DEBUG_PASSTHROUGH_ENABLED

void setup()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(ETHERNET_CS, OUTPUT);
  digitalWrite(ETHERNET_CS, HIGH);

  // Configure the LARA_PWR GPIO before enabling the regulators
  pinMode(LARA_PWR, OUTPUT);
  digitalWrite(LARA_PWR, LOW);

  // Now enable the 3.3V regulators for the GNSS and LARA
  pinMode(PWREN, OUTPUT);
  digitalWrite(PWREN, HIGH);
  
  pinMode(LARA_NI, INPUT);
  pinMode(LARA_ON, INPUT_PULLDOWN);
  
  delay(1000); // Wait for the ESP32

  Serial.begin(115200);
  Serial.println("SparkFun RTK EVK - Test Sketch");

  // The LARA_R6 will be powered off by default.
  // If desired, we can power it on manually by toggling the LARA_PWR pin now.
  // Or we can wait and let myLARA.begin do it.
  // else
  // {
  //   digitalWrite(LARA_PWR, HIGH);
  //   delay(100);
  //   digitalWrite(LARA_PWR, LOW);
    
  //   delay(8000); // Wait > 7 seconds for the LARA to begin
  // }

  //myLARA.enableDebugging();

  myLARA.invertPowerPin(true); //LARA_PWR is inverted

  Serial.println(F("Initializing the LARA_R6. This could take ~20 seconds."));

  // Initialize the LARA
  if (myLARA.begin(laraSerial, 115200) )
  {
    Serial.println(F("LARA-R6 connected!"));
  }
  else
  {
    Serial.println(F("Unable to communicate with the LARA."));
  }

  if (!digitalRead(LARA_ON))
    Serial.println(F("LARA-R6 failed to power on!"));

  Serial.print(F("Waiting for NI to go high"));
  int tries = 0;
  const int maxTries = 60;
  while ((tries < maxTries) && (digitalRead(LARA_NI) == LOW))
  {
    delay(1000);
    Serial.print(F("."));
    tries++;
  }
  Serial.println();
  if (tries == maxTries)
    Serial.println(F("NI didn't go high. Giving up..."));

  int opsAvailable;
  struct operator_stats ops[MAX_OPERATORS];
  String currentOperator = "";
  bool newConnection = true;

  while (Serial.available()) Serial.read();
  
  // First check to see if we're already connected to an operator:
  if (myLARA.getOperator(&currentOperator) == SARA_R5_SUCCESS) {
    Serial.print(F("Already connected to: "));
    Serial.println(currentOperator);
    // If already connected provide the option to type y to connect to new operator
    Serial.println(F("Press y to connect to a new operator, or any other key to continue.\r\n"));
    while (!Serial.available()) ;
    if (Serial.read() != 'y')
    {
      newConnection = false;
    }
    else
    {
      myLARA.deregisterOperator(); // Deregister from the current operator so we can connect to a new one
    }
    while (Serial.available()) Serial.read();
  }

  if (newConnection)
  {
    // Set MNO to either Verizon, T-Mobile, AT&T, Telstra, etc.
    // This will narrow the operator options during our scan later
    Serial.println(F("Setting mobile-network operator"));
    if (myLARA.setNetworkProfile(MOBILE_NETWORK_OPERATOR))
    {
      Serial.print(F("Set mobile network operator to "));
      Serial.println(MOBILE_NETWORK_STRINGS[convertOperatorNumber(MOBILE_NETWORK_OPERATOR)] + "\r\n");
    }
    else
    {
      Serial.println(F("Error setting MNO. Try cycling the power. Freezing..."));
      while (1) ;
    }
    
    // Wait for user to press button before initiating network scan.
    Serial.println(F("Press any key scan for networks.."));
    serialWait();

    Serial.println(F("Scanning for networks... This could take up to 3 minutes\r\n"));
    // myLARA.getOperators takes in a operator_stats struct pointer and max number of
    // structs to scan for, then fills up those objects with operator names and numbers
    opsAvailable = myLARA.getOperators(ops, MAX_OPERATORS); // This will block for up to 3 minutes

    if (opsAvailable > 0)
    {
      // Pretty-print operators we found:
      Serial.println("Found " + String(opsAvailable) + " operators:");
      printOperators(ops, opsAvailable);
      Serial.println(String(opsAvailable + 1) + ": use automatic selection");
      Serial.println();

      Serial.println(F("******************************************************"));
      Serial.println(F("* Important note:                                    *"));
      Serial.println(F("* Please select a LTE or UTRAN operator only.        *"));
      Serial.println(F("* Selecting GSM will cause high peak currents during *"));
      Serial.println(F("* transmit and could cause the power to brown-out.   *"));
      Serial.println(F("******************************************************"));
      Serial.println();

      // Wait until the user presses a key to initiate an operator connection
      Serial.println("Press 1-" + String(opsAvailable + 1) + " to select an operator.");
      char c = 0;
      bool selected = false;
      while (!selected) {
        while (!Serial.available()) ;
        delay(10); // Wait for more chars to arrive
        int selection = 0;
        c = Serial.read();
        if ((c >= '0') && (c <= '9'))
          selection = c - '0';
        if (Serial.available()) // Check for 2nd digit
        {
          c = Serial.read();
          if ((c >= '0') && (c <= '9'))
          {
            selection *= 10;
            selection += c - '0';
          }
        }
        if ((selection >= 1) && (selection <= (opsAvailable + 1))) {
          selected = true;
          Serial.println("Connecting to option " + String(selection));
          if (selection == (opsAvailable + 1))
          {
            if (myLARA.automaticOperatorSelection() == SARA_R5_SUCCESS)
            {
              Serial.println("Automatic operator selection: successful\r\n");
            }
            else
            {
              Serial.println(F("Automatic operator selection: error. Reset and try again, or try another network."));
            }
          }
          else
          {
            if (myLARA.registerOperator(ops[selection - 1]) == SARA_R5_SUCCESS)
            {
              Serial.println("Network " + ops[selection - 1].longOp + " registered\r\n");
            }
            else
            {
              Serial.println(F("Error connecting to operator. Reset and try again, or try another network."));
            }
          }
        }
      }
    }
    else
    {
      Serial.println(F("Did not find an operator. Double-check SIM and antenna, reset and try again, or try another network."));
      while (1) ;
    }
  }

  delay(2000);

  // At the very end print connection information
  printInfo();

  Serial.println(F("LARA_R5 will power off in 5 seconds - ready for the next example."));
}

void loop()
{
  // Loop provides a debugging interface.
  if (laraSerial.available()) {
    Serial.write((char) laraSerial.read());
  }
#ifdef DEBUG_PASSTHROUGH_ENABLED
  if (Serial.available()) {
    laraSerial.write((char) Serial.read());
  }
#endif

  static unsigned long loopStart = millis();
  static bool offSent = false;
  if ((millis() > (loopStart + 5000)) && (!offSent)) // 5 second timeout
  {
    Serial.println(F("Powering off the LARA_R5 - ready for the next example."));
    myLARA.modulePowerOff();
    offSent = true;
  }
}

void printInfo(void)
{
  String currentOperator = "";

  Serial.println(F("Connection info:"));
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
  }

  // Operator name or number
  if (myLARA.getOperator(&currentOperator) == SARA_R5_SUCCESS)
  {
    Serial.print(F("Operator: "));
    Serial.println(currentOperator);
  }

  // Received signal strength
  Serial.println("RSSI: " + String(myLARA.rssi()));
  Serial.println();
}

void printOperators(struct operator_stats * ops, int operatorsAvailable)
{
  for (int i = 0; i < operatorsAvailable; i++)
  {
    Serial.print(String(i + 1) + ": ");
    Serial.print(ops[i].longOp + " (" + String(ops[i].numOp) + ") - ");
    switch (ops[i].stat)
    {
    case 0:
      Serial.print(F("UNKNOWN"));
      break;
    case 1:
      Serial.print(F("AVAILABLE"));
      break;
    case 2:
      Serial.print(F("CURRENT"));
      break;
    case 3:
      Serial.print(F("FORBIDDEN"));
      break;
    }
    switch (ops[i].act)
    {
    case 0:
      Serial.print(F(" - GSM"));
      break;
    case 2:
      Serial.print(F(" - UTRAN"));
      break;
    case 3:
      Serial.print(F(" - GSM/GPRS with EDGE"));
      break;
    case 7:
      Serial.print(F(" - LTE")); // SARA-R5 only supports LTE
      break;
    }
    Serial.println();
  }
  Serial.println();
}

void serialWait()
{
  while (Serial.available()) Serial.read();
  while (!Serial.available()) ;
  delay(100);
  while (Serial.available()) Serial.read();
}
