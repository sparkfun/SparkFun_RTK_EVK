/*
  ESP32-WROVER-IE Pin Allocations:
  D0  : Boot + Boot Button
  D1  : Serial TX (CH340 RX)
  D2  : STAT LED
  D3  : Serial RX (CH340 TX)
  D4  : SD CS
  D5  : GNSS Time Pulse - via 74HC4066 switch and PWREN
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
  D25 : Serial2 RX - ZED-F9P TXO
  D26 : LARA Power On
  D27 : Ethernet Chip Select
  D32 : PWREN
  D33 : Serial2 TX - ZED-F9P RXI
  A34 : LARA Network Indicator
  A35 : Board Detect (3.0V)
  A36 : SD Card Detect
  A39 : Ethernet Interrupt
*/

const int STAT_LED = 2;
const int SD_CS = 4; // Chip select for the microSD card
const int GNSS_INT = 5; // ZED_F9P time pulse interrupt
const int SDA_2 = 12; // OLED
const int SERIAL1_TX = 13; // LARA_TXDI
const int SERIAL1_RX = 14; // LARA RXDO
const int SCL_2 = 15; // OLED
const int SDA_1 = 21; // ZED-F9P and NEO-D9S
const int SCL_1 = 22; // ZED-F9P and NEO-D9S
const int SERIAL2_RX = 25; // ZED-F9P TXO
const int LARA_PWR = 26; // LARA_PWR_ON - inverted - set LARA_PWR high to pull LARA_PWR_ON low
const int ETHERNET_CS = 27; // Chip select for the WizNet W5500
const int PWREN = 32; // 74HC4066 switch Enable - pull high to enable SCL2/SDA2 and GNSS_INT
const int SERIAL2_TX = 33; // ZED-F9P RXI
const int LARA_NI = 34; // LARA Network Indicator - only valid when the LARA is powered on
const int SD_PRESENT = 36; // microSD card card present - from the microSD socket switch
const int ETHERNET_INT = 39; // WizNet W5500 interrupt

#include <Wire.h>
TwoWire I2C_1 = TwoWire(0);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void initPins(void)
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

  I2C_1.begin((int)SDA_1, (int)SCL_1); // Start I2C
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

HardwareSerial *console = nullptr;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void initSerial(HardwareSerial &port, int baud)
{
  console = &port;

  console->begin(baud);
  console->println(F("SparkFun RTK EVK - Test Sketch"));
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void prettyPrintString(String theString) // Pretty-print a String in HEX and ASCII format
{
  int theLength = theString.length();

  console->println();
  console->print(F("String length is "));
  console->print(theLength);
  console->print(F(" (0x"));
  console->print(theLength, HEX);
  console->println(F(")"));
  console->println();

  for (int i = 0; i < theLength; i += 16)
  {
    if (i < 10000)
      console->print(F("0"));
    if (i < 1000)
      console->print(F("0"));
    if (i < 100)
      console->print(F("0"));
    if (i < 10)
      console->print(F("0"));
    console->print(i);

    console->print(F(" 0x"));

    if (i < 0x1000)
      console->print(F("0"));
    if (i < 0x100)
      console->print(F("0"));
    if (i < 0x10)
      console->print(F("0"));
    console->print(i, HEX);

    console->print(F(" "));

    int j;
    for (j = 0; ((i + j) < theLength) && (j < 16); j++)
    {
      if (theString[i + j] < 0x10)
        console->print(F("0"));
      console->print(theString[i + j], HEX);
      console->print(F(" "));
    }

    if (((i + j) == theLength) && (j < 16))
    {
      for (int k = 0; k < (16 - (theLength % 16)); k++)
      {
        console->print(F("   "));
      }
    }

    for (j = 0; ((i + j) < theLength) && (j < 16); j++)
    {
      if ((theString[i + j] >= 0x20) && (theString[i + j] <= 0x7E))
        console->write(theString[i + j]);
      else
        console->print(F("."));
    }

    console->println();
  }

  console->println();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Return true if a key has been pressed
bool keyPressed(char *c = nullptr); // Header
bool keyPressed(char *c)
{
  if (console->available()) // Check for a new key press
  {
    delay(100); // Wait for any more keystrokes to arrive
    if (c != nullptr)
      *c = console->read();      // Store the first key if desired
    while (console->available()) // Empty the serial buffer
      console->read();
    return (true);
  }

  return (false);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
