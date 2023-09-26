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

#include <Wire.h> // Needed for I2C to GNSS

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3

SFE_UBLOX_GNSS_SUPER theGNSS;

const int SD_CS = 4; // Chip select for the microSD card
const int ETHERNET_CS = 27; // Chip select for the WizNet 5500
const int PWREN = 32; // 3V3_SW and SDIO Enable
const int STAT_LED = 2;
const int SERIAL1_TX = 13;
const int SERIAL1_RX = 14;
const int SCL_1 = 22;
const int SDA_1 = 21;
const int SCL_2 = 15;
const int SDA_2 = 12;
const int ETHERNET_INT = 33;
const int GNSS_INT = 25;

TwoWire I2C_1 = TwoWire(0);

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

  //theGNSS.enableDebugging(Serial); // Uncomment this line to enable debug messages on Serial

  I2C_1.begin((int)SDA_1, (int)SCL_1);

  while (!theGNSS.begin(I2C_1)) // Start the GNSS on Wire
  {
    Serial.println("GNSS not detected. Retrying...");
  }

  theGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

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
  // Query module for HPPOSLLH data

  if (theGNSS.getHPPOSLLH()) // Returns true when fresh data is available
  {
    int32_t latitude = theGNSS.getHighResLatitude();
    int8_t latitudeHp = theGNSS.getHighResLatitudeHp();
    int32_t longitude = theGNSS.getHighResLongitude();
    int8_t longitudeHp = theGNSS.getHighResLongitudeHp();
    int32_t ellipsoid = theGNSS.getElipsoid();
    int8_t ellipsoidHp = theGNSS.getElipsoidHp();
    int32_t msl = theGNSS.getMeanSeaLevel();
    int8_t mslHp = theGNSS.getMeanSeaLevelHp();
    uint32_t accuracy = theGNSS.getHorizontalAccuracy();

    // Defines storage for the lat and lon as double
    double d_lat; // latitude
    double d_lon; // longitude

    // Assemble the high precision latitude and longitude
    d_lat = ((double)latitude) / 10000000.0; // Convert latitude from degrees * 10^-7 to degrees
    d_lat += ((double)latitudeHp) / 1000000000.0; // Now add the high resolution component (degrees * 10^-9 )
    d_lon = ((double)longitude) / 10000000.0; // Convert longitude from degrees * 10^-7 to degrees
    d_lon += ((double)longitudeHp) / 1000000000.0; // Now add the high resolution component (degrees * 10^-9 )

    float f_ellipsoid;
    float f_accuracy;

    // Calculate the height above ellipsoid in mm * 10^-1
    f_ellipsoid = (ellipsoid * 10) + ellipsoidHp;
    f_ellipsoid = f_ellipsoid / 10000.0; // Convert from mm * 10^-1 to m

    f_accuracy = accuracy / 10000.0; // Convert from mm * 10^-1 to m

    // Finally, do the printing
    Serial.print("Lat (deg): ");
    Serial.print(d_lat, 9);
    Serial.print(", Lon (deg): ");
    Serial.print(d_lon, 9);

    Serial.print(", Accuracy (m): ");
    Serial.print(f_accuracy, 4); // Print the accuracy with 4 decimal places

    Serial.print(", Altitude (m): ");
    Serial.print(f_ellipsoid, 4); // Print the ellipsoid with 4 decimal places

    Serial.println();
  }
}
