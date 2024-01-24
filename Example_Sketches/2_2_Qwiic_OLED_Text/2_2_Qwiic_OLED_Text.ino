/*
  SparkFun RTK Control Test Sketch: Qwiic OLED

  License: MIT. Please see LICENSE.md for more details

  Pins 12 and 15 are strapping pins
  If you are running this code on a WROVER breakout:
    Uncomment the #define DELAY_10s
    Disconnect the Qwiic OLED when uploading the code
    Connect it during the 10s delay.

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

//#define DELAY_10s // Uncomment this line if you are using a WROVER breakout with no strapping pin isolation

const int SD_CS = 4; // Chip select for the microSD card
const int ETHERNET_CS = 27; // Chip select for the WizNet 5500
const int PWREN = 32; // 3V3_SW and SDIO Enable
const int STAT_LED = 2;
const int SCL_2 = 15;
const int SDA_2 = 12;

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
  Serial.println("SparkFun RTK - Test Sketch");

#ifdef DELAY_10s
  Serial.print("Test starts in 10 seconds");
  for (int i = 0; i < 10; i++)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
#endif

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
