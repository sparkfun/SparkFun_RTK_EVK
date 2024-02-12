/*
  SparkFun RTK EVK Test Sketch

  License: MIT. Please see LICENSE.md for more details

  This example demonstrates how to power-on the LARA-R6 and check it is connected.

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

#include <HardwareSerial.h>
HardwareSerial laraSerial(1); //UART1

#include <SparkFun_u-blox_Cellular_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_u-blox_Cellular

// Derive the LARA_R6 class, so we can override beginSerial
class LARA_R6_Derived : public SparkFun_ublox_LARA_R6001D
{
  public:
    LARA_R6_Derived() : SparkFun_ublox_LARA_R6001D{LARA_PWR}{} // Pass the LARA_PWR pin into the class so the library can powerOn / powerOff

  protected:
    void beginSerial(unsigned long baud) override
    {
      delay(100);
      if (_hardSerial != nullptr)
      {
        _hardSerial->end();
        _hardSerial->begin(baud, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX); // Configure Serial1
      }
      delay(100);
    }
};

// Create a LARA_R6 object to use throughout the sketch
LARA_R6_Derived myLARA;

void setup()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  pinMode(ETHERNET_CS, OUTPUT);
  digitalWrite(ETHERNET_CS, HIGH);

  digitalWrite(LARA_PWR, LOW);
  pinMode(LARA_PWR, OUTPUT);
  digitalWrite(LARA_PWR, LOW);

  pinMode(PWREN, OUTPUT);
  digitalWrite(PWREN, HIGH);
  
  pinMode(LARA_NI, INPUT);
  
  delay(1000); // Wait for the ESP32

  Serial.begin(115200);
  Serial.println("SparkFun RTK EVK - Test Sketch");

  // The LARA_R6 will be powered off by default.
  // If desired, we can power it on manually by toggling the LARA_PWR pin now.
  // Or we can wait and let myLARA.begin do it.
  // {
  //   digitalWrite(LARA_PWR, HIGH);
  //   delay(100);
  //   digitalWrite(LARA_PWR, LOW);
    
  //   delay(8000); // Wait > 7 seconds for the LARA to begin
  // }

  myLARA.enableDebugging();

  myLARA.invertPowerPin(true); //LARA_PWR is inverted

  // Initialize the LARA
  if (myLARA.begin(laraSerial, 115200) )
  {
    Serial.println(F("LARA-R6 connected"));
  }
  else
  {
    Serial.println(F("Unable to communicate with the LARA!"));
  }

  Serial.println(F("LARA_R5 will power off in 30 seconds - ready for the next example"));
}

void loop()
{
  static unsigned long loopStart = millis();
  static bool offSent = false;
  if ((millis() > (loopStart + 30000)) && (!offSent)) // 30 second timeout
  {
    Serial.println(F("Powering off the LARA_R5 - ready for the next example"));
    myLARA.modulePowerOff();
    offSent = true;
  }

  if (!offSent)
  {
    Serial.print(F("LARA Network Indicator (NI) pin is: "));
    if (digitalRead(LARA_NI) == HIGH)
      Serial.println(F("HIGH"));
    else
      Serial.println(F("LOW"));
  }

  delay(1000);
}
