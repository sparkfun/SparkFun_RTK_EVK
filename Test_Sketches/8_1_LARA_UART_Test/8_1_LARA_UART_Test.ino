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
//SARA_R5 mySARA(LARA_PWR);

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
  
  myLARA.enableDebugging();

  myLARA.invertPowerPin(false); //LARA_PWR is not inverted. But probably should be...
  
  // Initialize the LARA - this checks the custom TX and RX pins don't get reset when _hardSerial->begin(baud); is called internally
  if (myLARA.begin(laraSerial, 115200) )
  {
    Serial.println(F("LARA-R6 connected!"));
  }
  else
  {
    Serial.println(F("Unable to communicate with the LARA."));
  }
}

void loop()
{
  //Nothing to do here...
}
