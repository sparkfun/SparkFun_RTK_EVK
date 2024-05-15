/*
  SparkFun RTK EVK Test Sketch

  W5500 Ethernet using the Espressif ESP32 ETH. Based on:
  https://github.com/espressif/arduino-esp32/blob/master/libraries/Ethernet/examples/ETH_W5500_Arduino_SPI/ETH_W5500_Arduino_SPI.ino

  Select ESP32 Wrover as the board

  NOTE: this requires arduino-esp32 v3.0.0

  Tested with v3.0.0 RC2:
  https://github.com/espressif/arduino-esp32/releases/tag/3.0.0-rc2

  See: https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html
  Add this link to the Additional Boards Manager URLs: https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json

  See also:
  https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html
  https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#functional-changes

  Prints updates to the OLED - to ensure simultaneous SPI and I2C is possible

  Connects the STAT LED to the GNSS Time Pulse - to ensure Arduino interrupts still work correctly

  Accesses the microSD using SdFat - to ensure simultaneous Arduino SPI is possible

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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// I2C OLED

#include <Wire.h>
TwoWire I2C_2 = TwoWire(1);

#include <SparkFun_Qwiic_OLED.h> //http://librarymanager/All#SparkFun_Qwiic_OLED
Qwiic1in3OLED myOLED;            // 128x64

// Fonts
#include <res/qw_fnt_5x7.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// arduino-esp32 Ethernet

#include <SPI.h>
#include <ETH.h>

const int STAT_LED = 2;
const int SD_CS = 4;         // Chip select for the microSD card
const int GNSS_INT = 5;      // ZED_F9P time pulse interrupt
const int SDA_2 = 12;        // OLED
const int SERIAL1_TX = 13;   // LARA_TXDI
const int SERIAL1_RX = 14;   // LARA RXDO
const int SCL_2 = 15;        // OLED
const int SDA_1 = 21;        // ZED-F9P and NEO-D9S
const int SCL_1 = 22;        // ZED-F9P and NEO-D9S
const int SERIAL2_RX = 25;   // ZED-F9P TXO
const int LARA_PWR = 26;     // LARA_PWR_ON - inverted - set LARA_PWR high to pull LARA_PWR_ON low
const int ETHERNET_CS = 27;  // Chip select for the WizNet W5500
const int PWREN = 32;        // 74HC4066 switch Enable - pull high to enable SCL2/SDA2 and GNSS_INT
const int SERIAL2_TX = 33;   // ZED-F9P RXI
const int LARA_NI = 34;      // LARA Network Indicator - only valid when the LARA is powered on
const int SD_PRESENT = 36;   // microSD card card present - from the microSD socket switch
const int ETHERNET_INT = 39; // WizNet W5500 interrupt

#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 0
#define ETH_PHY_CS ETHERNET_CS
#define ETH_PHY_IRQ ETHERNET_INT
#define ETH_PHY_RST -1

#define ETH_SPI_SCK 18
#define ETH_SPI_MISO 19
#define ETH_SPI_MOSI 23

static bool eth_connected = false;

void onEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-eth0");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.printf("ETH Got IP: '%s'\n", esp_netif_get_desc(info.got_ip.esp_netif));
        Serial.println(ETH);
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_LOST_IP:
        Serial.println("ETH Lost IP");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}

void testClient(const char *host, uint16_t port)
{
    Serial.print("\nConnecting to ");
    Serial.println(host);

    myOLED.erase();

    myOLED.setFont(QW_FONT_5X7);
    myOLED.text(0, 0, "Connecting to");
    myOLED.text(0, 8, host);
    myOLED.display();

    NetworkClient client;
    if (!client.connect(host, port))
    {
        Serial.println("Connection failed");
        myOLED.text(0, 16, "Connection failed");
        myOLED.display();
        return;
    }

    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);

    while (client.connected() && !client.available())
        ;

    if (client.available())
    {
        myOLED.text(0, 16, "Client connected");
        myOLED.display();
    }

    // Test SdFat microSD now - while ETH is ~busy
    int files = countFiles();
    Serial.printf("\nFiles found: %d\n\n", files);
    char printMe[21];
    snprintf(printMe, sizeof(printMe), "Files found: %d", files);
    myOLED.text(0, 48, printMe);
    myOLED.display();

    while (client.available())
    {
        Serial.write(client.read());
    }

    Serial.println("Closing connection\n");
    myOLED.text(0, 24, "Closing connection");
    myOLED.display();

    client.stop();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void tpISR()
{
    digitalWrite(STAT_LED, digitalRead(GNSS_INT)); // Make the LED follow TP
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// SD access using SdFat

#include "SdFat.h"

SdFat *sd = nullptr;

// Make size of files human readable
void stringHumanReadableSize(String &returnText, uint64_t bytes)
{
    char suffix[5] = {'\0'};
    char readableSize[50] = {'\0'};
    float cardSize = 0.0;

    if (bytes < 1024)
        strcpy(suffix, "B");
    else if (bytes < (1024 * 1024))
        strcpy(suffix, "KB");
    else if (bytes < (1024 * 1024 * 1024))
        strcpy(suffix, "MB");
    else
        strcpy(suffix, "GB");

    if (bytes < (1024))
        cardSize = bytes; // B
    else if (bytes < (1024 * 1024))
        cardSize = bytes / 1024.0; // KB
    else if (bytes < (1024 * 1024 * 1024))
        cardSize = bytes / 1024.0 / 1024.0; // MB
    else
        cardSize = bytes / 1024.0 / 1024.0 / 1024.0; // GB

    if (strcmp(suffix, "GB") == 0)
        snprintf(readableSize, sizeof(readableSize), "%0.1f %s", cardSize, suffix); // Print decimal portion
    else if (strcmp(suffix, "MB") == 0)
        snprintf(readableSize, sizeof(readableSize), "%0.1f %s", cardSize, suffix); // Print decimal portion
    else if (strcmp(suffix, "KB") == 0)
        snprintf(readableSize, sizeof(readableSize), "%0.1f %s", cardSize, suffix); // Print decimal portion
    else
        snprintf(readableSize, sizeof(readableSize), "%.0f %s", cardSize, suffix); // Don't print decimal portion

    returnText = String(readableSize);
}

// Create a test file in file structure to make sure we can
bool createTestFile()
{
    SdFile testFile;

    // TODO: double-check that SdFat tollerates preceding slashes
    char testFileName[40] = "/testfile.txt";

    unsigned long startTime = millis();

    // Attempt to write to the file system
    if (testFile.open(testFileName, O_CREAT | O_APPEND | O_WRITE) != true)
    {
        Serial.println("createTestFile: failed to create (open) test file");
        return (false);
    }

    for (int i = 0; i < 1048576; i++)
        testFile.write(uint8_t(i & 0xFF));

    // File successfully created
    testFile.close();

    unsigned long endTime = millis();

    Serial.printf("createTestFile: wrote 1048576 bytes in %ldms\r\n", endTime - startTime);

    startTime = millis();

    // Attempt to read from the file system
    if (testFile.open(testFileName, O_READ) != true)
    {
        Serial.println("createTestFile: failed to open test file for reading");
        return (false);
    }

    for (int i = 0; i < 1048576; i++)
    {
        uint8_t c;
        testFile.read(&c, 1);
        if (c != (i & 0xFF))
            Serial.println("File read error!");
    }

    testFile.close();

    endTime = millis();

    Serial.printf("createTestFile: read 1048576 bytes in %ldms\r\n", endTime - startTime);

    SdFile dir;
    dir.open("/"); // Open root
    uint16_t fileCount = 0;

    SdFile tempFile;

    Serial.println("Files found:");

    while (tempFile.openNext(&dir, O_READ))
    {
        if (tempFile.isFile())
        {
            fileCount++;

            // 2017-05-19 187362648 800_0291.MOV

            // Get File Date from sdFat
            uint16_t fileDate;
            uint16_t fileTime;
            tempFile.getCreateDateTime(&fileDate, &fileTime);

            // Convert sdFat file date format into YYYY-MM-DD
            char fileDateChar[20];
            snprintf(fileDateChar, sizeof(fileDateChar), "%d-%02d-%02d",
                     ((fileDate >> 9) + 1980),   // Year
                     ((fileDate >> 5) & 0b1111), // Month
                     (fileDate & 0b11111)        // Day
            );

            char fileSizeChar[20];
            String fileSize;
            stringHumanReadableSize(fileSize, tempFile.fileSize());
            fileSize.toCharArray(fileSizeChar, sizeof(fileSizeChar));

            char fileName[50]; // Handle long file names
            tempFile.getName(fileName, sizeof(fileName));

            char fileRecord[100];
            snprintf(fileRecord, sizeof(fileRecord), "%s\t%s\t%s", fileDateChar, fileSizeChar, fileName);

            Serial.println(fileRecord);
        }
    }

    dir.close();
    tempFile.close();

    if (fileCount == 0)
        Serial.println("No files found");

    if (sd->exists(testFileName))
        sd->remove(testFileName);

    return (!sd->exists(testFileName));
}

// Count the files in the file structure to make sure we can
int countFiles()
{
    SdFile dir;
    dir.open("/"); // Open root
    int fileCount = 0;

    SdFile tempFile;

    while (tempFile.openNext(&dir, O_READ))
    {
        if (tempFile.isFile())
        {
            fileCount++;
        }
    }

    dir.close();
    tempFile.close();

    return (fileCount);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    pinMode(ETHERNET_CS, OUTPUT);
    digitalWrite(ETHERNET_CS, HIGH);
    pinMode(GNSS_INT, INPUT);
    pinMode(STAT_LED, OUTPUT);
    digitalWrite(STAT_LED, HIGH);
    pinMode(PWREN, OUTPUT);
    digitalWrite(PWREN, HIGH);

    delay(1000);

    Serial.begin(115200);
    Serial.println("SparkFun RTK EVK - Test Sketch");

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

    I2C_2.begin((int)SDA_2, (int)SCL_2, (uint32_t)400000);

    // Initalize the OLED device and related graphics system
    if (myOLED.begin(I2C_2) == false)
    {
        Serial.println("OLED begin failed...");
        return;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

    attachInterrupt(GNSS_INT, tpISR, CHANGE);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Allocate the data structure that manages the microSD card

    if (!sd)
    {
        sd = new SdFat();
        if (!sd)
        {
            Serial.println("Failed to allocate the SdFat structure!");
            return;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Begin SdFat microSD communication
    
    if (sd->begin(SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(16))) == false)
    {
        int tries3 = 0;
        int maxTries3 = 2;
        for (; tries3 < maxTries3; tries3++)
        {
            Serial.printf("SD init failed - using SPI and SdFat. Trying again %d out of %d\r\n", tries3 + 1, maxTries3);

            delay(250); // Give SD more time to power up, then try again
            if (sd->begin(SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(16))) == true)
                return;
        }

        if (tries3 == maxTries3)
        {
            Serial.println("SD init failed - using SPI and SdFat. Is card formatted?");
            digitalWrite(SD_CS, HIGH); // Be sure SD is deselected

            return;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Access the file system using SdFat
    
    // Change to root directory. All new file creation will be in root.
    if (sd->chdir() == false)
    {
        Serial.println("SD change directory failed");
    }

    if (createTestFile())
        Serial.println("\nmicroSD + SdFat : success\n");
    else
        Serial.println("\nmicroSD + SdFat : FAIL\n");

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // (Re)Begin SPI

    SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Begin ETHernet

    Network.onEvent(onEvent);

    ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI);

}

void loop()
{
    if (eth_connected)
    {
        testClient("google.com", 80);
    }
    delay(10000);
}
