/*
  SparkFun RTK EVK Test Sketch

  License: MIT. Please see LICENSE.md for more details

  ESP32-WROVER-IE Pin Allocations:
  D0  : Boot + Boot Button
  D1  : Serial TX (CH340 RX)
  D2  : STAT LED
  D3  : Serial RX (CH340 TX)
  D4  : SD CS
  D5  : LARA_ON - via 74HC4066 switch and PWREN
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
const int SCL_2 = 15; // OLED
const int SDA_2 = 12; // OLED
const int ETHERNET_CS = 27; // Chip select for the WizNet 5500
const int PWREN = 32; // 74HC4066 switch Enable - pull high to enable SCL2/SDA2 and LARA_ON

#include <Wire.h>
TwoWire I2C_2 = TwoWire(1);

#include <SparkFun_Qwiic_OLED.h> //http://librarymanager/All#SparkFun_Qwiic_OLED
Qwiic1in3OLED myOLED; // 128x64

// Fonts
#include <res/qw_fnt_5x7.h>
#include <res/qw_fnt_8x16.h>
#include <res/qw_fnt_31x48.h>
#include <res/qw_fnt_7segment.h>
#include <res/qw_fnt_largenum.h>

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
  Serial.println("SparkFun RTK EVK - Test Sketch");

  I2C_2.begin((int)SDA_2, (int)SCL_2, (uint32_t)400000);

  // Initalize the OLED device and related graphics system
  if (myOLED.begin(I2C_2) == false)
  {
      Serial.println("OLED begin failed. Freezing...");
      while (true)
          ;
  }

  myOLED.erase();
  
  myOLED.setFont(QW_FONT_5X7);
  myOLED.text(0,0,"01234567");

  myOLED.setFont(QW_FONT_5X7);
  myOLED.text(0,8,"ABCDabcd");

  myOLED.setFont(QW_FONT_8X16);
  myOLED.text(48,0,"ABcd");

  myOLED.setFont(QW_FONT_8X16);
  myOLED.text(48,13,"efGH");

  myOLED.setFont(QW_FONT_7SEGMENT);
  myOLED.text(80,0,"0123");

  myOLED.setFont(QW_FONT_5X7);
  myOLED.text(80,17,"ijkLMNO");

  myOLED.setFont(QW_FONT_LARGENUM);
  myOLED.text(0,16,"012");

  myOLED.setFont(QW_FONT_31X48);
  myOLED.text(36,27,"ABC");

  myOLED.display();
}

void loop()
{
}
