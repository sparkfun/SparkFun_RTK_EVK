/*
  SparkFun RTK EVK Test Sketch

  License: MIT. Please see LICENSE.md for more details

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

extern class SARA_R5_DERIVED myLARA;

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

  public:
    SARA_R5_error_t factoryConfiguration(uint8_t fs_op, uint8_t nvm_op)
    {
      SARA_R5_error_t err;
      char *command;
    
      command = (char *)calloc(sizeof("+UFACTORY=") + 4, sizeof(char));
      if (command == nullptr)
        return SARA_R5_ERROR_OUT_OF_MEMORY;
      sprintf(command, "%s%d,%d", "+UFACTORY=", fs_op, nvm_op);
      err = sendCommandWithResponse(command, SARA_R5_RESPONSE_OK,
                                    nullptr, SARA_R5_STANDARD_RESPONSE_TIMEOUT);
      free(command);
      return err;
    }
};

// Create a LARA_R6 object to use throughout the sketch
SARA_R5_DERIVED myLARA;

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

  // Send the factory configuration: clear file system, delete modem profiles
  if (myLARA.factoryConfiguration(0,1) == SARA_R5_ERROR_SUCCESS)
    Serial.println(F("Factory configuration sent. Reboot module to apply."));
  else
    Serial.println(F("Factory configuration failed!"));

  Serial.println(F("Powering off the LARA"));

  myLARA.modulePowerOff();
}

void loop()
{
}
