/*
  SparkFun RTK EVK Test Sketch

  License: MIT. Please see LICENSE.md for more details

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

  public:
    UBX_CELL_error_t factoryConfiguration(uint8_t fs_op, uint8_t nvm_op)
    {
      UBX_CELL_error_t err;
      char *command;
    
      command = (char *)calloc(sizeof("+UFACTORY=") + 4, sizeof(char));
      if (command == nullptr)
        return UBX_CELL_ERROR_OUT_OF_MEMORY;
      sprintf(command, "%s%d,%d", "+UFACTORY=", fs_op, nvm_op);
      err = sendCommandWithResponse(command, UBX_CELL_RESPONSE_OK,
                                    nullptr, UBX_CELL_STANDARD_RESPONSE_TIMEOUT);
      free(command);
      return err;
    }
};

// Create a LARA_R6 object to use throughout the sketch
LARA_R6_Derived myLARA;

// Uncomment this line if you want to be able to communicate directly with the LARA in the main loop
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

  // Send the factory configuration: clear file system, delete modem profiles
  if (myLARA.factoryConfiguration(0,1) == UBX_CELL_SUCCESS)
    Serial.println(F("Factory configuration sent. Reboot module to apply."));
  else
    Serial.println(F("Factory configuration failed!"));

  Serial.println(F("Powering off the LARA"));

  myLARA.modulePowerOff();
}

void loop()
{
}
