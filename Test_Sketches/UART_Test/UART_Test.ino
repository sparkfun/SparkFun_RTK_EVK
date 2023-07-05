/*
  SparkFun RTK Control Test Sketch: Hardware Serial on custom pins

  License: MIT. Please see LICENSE.md for more details

  Connect a u-blox GNSS to pins 13 and 14

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

#include <HardwareSerial.h>
HardwareSerial mySerial(2); //UART2 normally uses pins 16 and 17, but these are not available on WROVER

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3

SFE_UBLOX_GNSS_SERIAL theGNSS;

void setup()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(ETHERNET_CS, OUTPUT);
  digitalWrite(ETHERNET_CS, HIGH);
  pinMode(PWREN, OUTPUT);
  digitalWrite(PWREN, HIGH);

  delay(1000);

  Serial.begin(115200);
  Serial.println("SparkFun RTK - Test Sketch");
  
  mySerial.begin(38400, SERIAL_8N1, SERIAL_RX, SERIAL_TX); // Configure Serial1

  while (!theGNSS.begin(mySerial)) // Start the GNSS
  {
    Serial.println("GNSS not detected. Retrying...");
  }

  // Query module info

  if (theGNSS.getModuleInfo())
  {
      Serial.print(F("The GNSS module is: "));
      Serial.println(theGNSS.getModuleName());    

      Serial.print(F("The firmware type is: "));
      Serial.println(theGNSS.getFirmwareType());    

      Serial.print(F("The firmware version is: "));
      Serial.print(theGNSS.getFirmwareVersionHigh());
      Serial.print(F("."));
      Serial.println(theGNSS.getFirmwareVersionLow());
      
      Serial.print(F("The protocol version is: "));
      Serial.print(theGNSS.getProtocolVersionHigh());
      Serial.print(F("."));
      Serial.println(theGNSS.getProtocolVersionLow());
  }  
}

void loop()
{
    theGNSS.checkUblox();
}
