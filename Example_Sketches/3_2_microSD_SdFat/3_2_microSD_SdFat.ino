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

#include "SPI.h"
#include "SdFat.h"

const int STAT_LED = 2;
const int pin_microSD_CS = 4; // Chip select for the microSD card
const int SDA_2 = 12; // OLED
const int SERIAL1_TX = 13; // LARA_TXDI
const int SERIAL1_RX = 14; // LARA RXDO
const int SCL_2 = 15; // OLED
const int ETHERNET_CS = 27; // Chip select for the WizNet W5500
const int PWREN = 32; // 74HC4066 switch Enable - pull high to enable SCL2/SDA2 and LARA_ON
const int ETHERNET_INT = 33; // WizNet W5500 interrupt
const int GNSS_INT = 25; // ZED_F9P interrupt
const int SD_PRESENT = 36; // microSD card card present - from the microSD socket switch

SdFat *sd;

// Attempt to de-init the SD card - SPI only
// https://github.com/greiman/SdFat/issues/351
void resetSPI()
{
    {
        pinMode(pin_microSD_CS, OUTPUT);
        digitalWrite(pin_microSD_CS, HIGH); // De-select SD card

        // Flush SPI interface
        SPI.begin();
        SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
        for (int x = 0; x < 10; x++)
            SPI.transfer(0XFF);
        SPI.endTransaction();
        SPI.end();

        digitalWrite(pin_microSD_CS, LOW); // Select SD card

        // Flush SD interface
        SPI.begin();
        SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
        for (int x = 0; x < 10; x++)
            SPI.transfer(0XFF);
        SPI.endTransaction();
        SPI.end();

        digitalWrite(pin_microSD_CS, HIGH); // Deselet SD card
    }
}

// Define commands for the SD card
#define SD_GO_IDLE (0x40 + 0)      // CMD0 - go to idle state
#define SD_INIT (0x40 + 1)         // CMD1 - start initialization
#define SD_SEND_IF_COND (0x40 + 8) // CMD8 - send interface (conditional), works for SDHC only
#define SD_SEND_STATUS (0x40 + 13) // CMD13 - send card status
#define SD_SET_BLK_LEN (0x40 + 16) // CMD16 - set length of block in bytes
#define SD_LOCK_UNLOCK (0x40 + 42) // CMD42 - lock/unlock card
#define CMD55 (0x40 + 55)          // multi-byte preface command
#define SD_READ_OCR (0x40 + 58)    // read OCR
#define SD_ADV_INIT (0xc0 + 41)    // ACMD41, for SDHC cards - advanced start initialization

// Define options for accessing the SD card's PWD (CMD42)
#define MASK_ERASE 0x08       // erase the entire card
#define MASK_LOCK_UNLOCK 0x04 // lock or unlock the card with password
#define MASK_CLR_PWD 0x02     // clear password
#define MASK_SET_PWD 0x01     // set password

// Define bit masks for fields in the lock/unlock command (CMD42) data structure
#define SET_PWD_MASK (1 << 0)
#define CLR_PWD_MASK (1 << 1)
#define LOCK_UNLOCK_MASK (1 << 2)
#define ERASE_MASK (1 << 3)

// Begin initialization by sending CMD0 and waiting until SD card
// responds with In Idle Mode (0x01). If the response is not 0x01
// within a reasonable amount of time, there is no SD card on the bus.
// Returns false if not card is detected
// Returns true if a card responds
bool sdPresent(void)
{
    {
        byte response = 0;

        SPI.begin();
        SPI.setClockDivider(SPI_CLOCK_DIV2);
        SPI.setDataMode(SPI_MODE0);
        SPI.setBitOrder(MSBFIRST);
        pinMode(pin_microSD_CS, OUTPUT);

        // Sending clocks while card power stabilizes...
        deselectCard();               // always make sure
        for (byte i = 0; i < 30; i++) // send several clocks while card power stabilizes
            xchg(0xff);

        // Sending CMD0 - GO IDLE...
        for (byte i = 0; i < 0x10; i++) // Attempt to go idle
        {
            response = sdSendCommand(SD_GO_IDLE, 0); // send CMD0 - go to idle state
            if (response == 1)
                break;
        }
        if (response != 1)
            return (false); // Card failed to respond to idle
    }

    return (true);
}

/*
    sdSendCommand      send raw command to SD card, return response

    This routine accepts a single SD command and a 4-byte argument.  It sends
    the command plus argument, adding the appropriate CRC.  It then returns
    the one-byte response from the SD card.

    For advanced commands (those with a command byte having bit 7 set), this
    routine automatically sends the required preface command (CMD55) before
    sending the requested command.

    Upon exit, this routine returns the response byte from the SD card.
    Possible responses are:
      0xff  No response from card; card might actually be missing
      0x01  SD card returned 0x01, which is OK for most commands
      0x??  other responses are command-specific
*/
byte sdSendCommand(byte command, unsigned long arg)
{
    byte response;

    if (command & 0x80) // special case, ACMD(n) is sent as CMD55 and CMDn
    {
        command &= 0x7f;                    // strip high bit for later
        response = sdSendCommand(CMD55, 0); // send first part (recursion)
        if (response > 1)
            return (response);
    }

    deselectCard();
    xchg(0xFF);
    selectCard(); // enable CS
    xchg(0xFF);

    xchg(command | 0x40);    // command always has bit 6 set!
    xchg((byte)(arg >> 24)); // send data, starting with top byte
    xchg((byte)(arg >> 16));
    xchg((byte)(arg >> 8));
    xchg((byte)(arg & 0xFF));

    byte crc = 0x01; // good for most cases
    if (command == SD_GO_IDLE)
        crc = 0x95; // this will be good enough for most commands
    if (command == SD_SEND_IF_COND)
        crc = 0x87; // special case, have to use different CRC
    xchg(crc);      // send final byte

    for (int i = 0; i < 30; i++) // loop until timeout or response
    {
        response = xchg(0xFF);
        if ((response & 0x80) == 0)
            break; // high bit cleared means we got a response
    }

    /*
        We have issued the command but the SD card is still selected.  We
        only deselectCard the card if the command we just sent is NOT a command
        that requires additional data exchange, such as reading or writing
        a block.
    */
    if ((command != SD_READ_OCR) && (command != SD_SEND_STATUS) && (command != SD_SEND_IF_COND) &&
        (command != SD_LOCK_UNLOCK))
    {
        deselectCard(); // all done
        xchg(0xFF);     // close with eight more clocks
    }

    return (response); // let the caller sort it out
}

// Select (enable) the SD card
void selectCard(void)
{
    digitalWrite(pin_microSD_CS, LOW);
}

// Deselect (disable) the SD card
void deselectCard(void)
{
    digitalWrite(pin_microSD_CS, HIGH);
}

// Exchange a byte of data with the SD card via host's SPI bus
byte xchg(byte val)
{
    byte receivedVal = SPI.transfer(val);
    return receivedVal;
}

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

    {
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

                // Convert sdFat file date fromat into YYYY-MM-DD
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
    }

    {
        if (sd->exists(testFileName))
            sd->remove(testFileName);
        return (!sd->exists(testFileName));
    }

    return (false);
}

void setup()
{
  pinMode(pin_microSD_CS, OUTPUT);
  digitalWrite(pin_microSD_CS, HIGH);
  pinMode(ETHERNET_CS, OUTPUT);
  digitalWrite(ETHERNET_CS, HIGH);
  pinMode(PWREN, OUTPUT);
  digitalWrite(PWREN, HIGH);
  pinMode(SD_PRESENT, INPUT_PULLUP);

  delay(1000);

  Serial.begin(115200);
  Serial.println("SparkFun RTK EVK - Test Sketch");

  Serial.println("Initializing microSD - using SPI, SdFat and SdFile");

  while (digitalRead(SD_PRESENT)){ // SD_PRESENT is high when the card is not present
    Serial.println("Card not present. Please insert one to begin the test...");
    delay(1000);
  }

  resetSPI();                         // Re-initialize the SPI/SD interface

  // Do a quick test to see if a card is present
  int tries = 0;
  int maxTries = 5;
  while (tries < maxTries)
  {
      if (sdPresent() == true)
          break;
      Serial.printf("SD present failed. Trying again %d out of %d\r\n", tries + 1, maxTries);

      // Max power up time is 250ms: https://www.kingston.com/datasheets/SDCIT-specsheet-64gb_en.pdf
      // Max current is 200mA average across 1s, peak 300mA
      delay(10);
      tries++;
  }
  if (tries == maxTries)
      return; // Give up loop

  // If an SD card is present, allow SdFat to take over
  Serial.println("SD card detected - using SPI and SdFat");

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

  resetSPI(); // Re-initialize the SPI/SD interface

  if (sd->begin(SdSpiConfig(pin_microSD_CS, SHARED_SPI, SD_SCK_MHZ(16))) == false)
  {
      tries = 0;
      maxTries = 1;
      for (; tries < maxTries; tries++)
      {
          Serial.printf("SD init failed - using SPI and SdFat. Trying again %d out of %d\r\n", tries + 1, maxTries);

          delay(250); // Give SD more time to power up, then try again
          if (sd->begin(SdSpiConfig(pin_microSD_CS, SHARED_SPI, SD_SCK_MHZ(16))) == true)
              return;
      }

      if (tries == maxTries)
      {
          Serial.println("SD init failed - using SPI and SdFat. Is card formatted?");
          digitalWrite(pin_microSD_CS, HIGH); // Be sure SD is deselected

          return;
      }
  }

  // Change to root directory. All new file creation will be in root.
  if (sd->chdir() == false)
  {
      Serial.println("SD change directory failed");
  }

  if (createTestFile() == false)
  {
      Serial.println("Failed to create test file. Format SD card with 'SD Card Formatter'.");
  }
  else
    Serial.println("microSD: Online");
}

void loop()
{
  // Nothing to do here...
}
