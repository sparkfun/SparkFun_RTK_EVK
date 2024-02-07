#include <Wire.h>
TwoWire I2C_1 = TwoWire(0);

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
SFE_UBLOX_GNSS myGNSS;

int myLat = 99999; // Store the latitude and longitude in centidegrees for localized distribution
int myLon = 99999;
int myTileLat = 99999; // The center of the localized tile in centidegrees
int myTileLon = 99999;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Callback: printPVTdata will be called when new NAV PVT data arrives
// See u-blox_structs.h for the full definition of UBX_NAV_PVT_data_t
//         _____  You can use any name you like for the callback. Use the same name when you call setAutoPVTcallback
//        /                  _____  This _must_ be UBX_NAV_PVT_data_t
//        |                 /               _____ You can use any name you like for the struct
//        |                 |              /
//        |                 |              |
void printPVTdata(UBX_NAV_PVT_data_t *ubxDataStruct)
{
  uint8_t fixType = ubxDataStruct->fixType;

  float tileDelta = 2.5; // 2.5 centidegrees (10 degrees and 5 degrees are also possible)
  if (localizedLevel == '0')
    tileDelta = 10.0;
  if (localizedLevel == '1')
    tileDelta = 5.0;

  double latitude = ubxDataStruct->lat; // Print the latitude
  console->print(F("Lat: "));
  console->print(latitude / 10000000.0, 7);
  if (fixType == 3)
  {
    myLat = latitude / 100000; // Latitude in centidegrees
    float lat = latitude / 10000000.0;
    lat = floor(lat / tileDelta) * tileDelta; // Calculate the tile center in centidegrees
    lat += tileDelta / 2.0;
    myTileLat = round(lat * 100.0);
  }

  double longitude = ubxDataStruct->lon; // Print the longitude
  console->print(F("  Long: "));
  console->print(longitude / 10000000.0, 7);
  if (fixType == 3)
  {
    myLon = longitude / 100000; // Longitude in centidegrees
    float lon = longitude / 10000000.0;
    lon = floor(lon / tileDelta) * tileDelta; // Calculate the tile center in centidegrees
    lon += tileDelta / 2.0;
    myTileLon = round(lon * 100.0);
  }

  double altitude = ubxDataStruct->hMSL; // Print the height above mean sea level
  console->print(F("  Height: "));
  console->print(altitude / 1000.0, 3);

  console->print(F("  Fix: ")); // Print the fix type
  console->print(fixType);
  if (fixType == 0)
    console->print(F(" (None)"));
  else if (fixType == 1)
    console->print(F(" (Dead Reckoning)"));
  else if (fixType == 2)
    console->print(F(" (2D)"));
  else if (fixType == 3)
    console->print(F(" (3D)"));
  else if (fixType == 4)
    console->print(F(" (GNSS + Dead Reckoning)"));
  else if (fixType == 5)
    console->print(F(" (Time Only)"));
  else
    console->print(F(" (UNKNOWN)"));

  uint8_t carrSoln = ubxDataStruct->flags.bits.carrSoln; // Print the carrier solution
  console->print(F("  Carrier Solution: "));
  console->print(carrSoln);
  if (carrSoln == 0)
    console->print(F(" (None)"));
  else if (carrSoln == 1)
    console->print(F(" (Floating)"));
  else if (carrSoln == 2)
    console->print(F(" (Fixed)"));
  else
    console->print(F(" (UNKNOWN)"));

  uint32_t hAcc = ubxDataStruct->hAcc; // Print the horizontal accuracy estimate
  console->print(F("  Horizontal Accuracy Estimate: "));
  console->print(hAcc);
  console->print(F(" (mm)"));

  console->println();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Callback: printRXMCOR will be called when new RXM COR data arrives
// See u-blox_structs.h for the full definition of UBX_RXM_COR_data_t
//         _____  You can use any name you like for the callback. Use the same name when you call setRXMCORcallbackPtr
//        /                  _____  This _must_ be UBX_RXM_COR_data_t
//        |                 /               _____ You can use any name you like for the struct
//        |                 |              /
//        |                 |              |
void printRXMCOR(UBX_RXM_COR_data_t *ubxDataStruct)
{
  console->print(F("UBX-RXM-COR:  ebno: "));
  console->print((double)ubxDataStruct->ebno / 8, 3); // Convert to dB

  console->print(F("  protocol: "));
  if (ubxDataStruct->statusInfo.bits.protocol == 1)
    console->print(F("RTCM3"));
  else if (ubxDataStruct->statusInfo.bits.protocol == 2)
    console->print(F("SPARTN"));
  else if (ubxDataStruct->statusInfo.bits.protocol == 29)
    console->print(F("PMP (SPARTN)"));
  else if (ubxDataStruct->statusInfo.bits.protocol == 30)
    console->print(F("QZSSL6"));
  else
    console->print(F("Unknown"));

  console->print(F("  errStatus: "));
  if (ubxDataStruct->statusInfo.bits.errStatus == 1)
    console->print(F("Error-free"));
  else if (ubxDataStruct->statusInfo.bits.errStatus == 2)
    console->print(F("Erroneous"));
  else
    console->print(F("Unknown"));

  console->print(F("  msgUsed: "));
  if (ubxDataStruct->statusInfo.bits.msgUsed == 1)
    console->print(F("Not used"));
  else if (ubxDataStruct->statusInfo.bits.msgUsed == 2)
    console->print(F("Used"));
  else
    console->print(F("Unknown"));

  console->print(F("  msgEncrypted: "));
  if (ubxDataStruct->statusInfo.bits.msgEncrypted == 1)
    console->print(F("Not encrypted"));
  else if (ubxDataStruct->statusInfo.bits.msgEncrypted == 2)
    console->print(F("Encrypted"));
  else
    console->print(F("Unknown"));

  console->print(F("  msgDecrypted: "));
  if (ubxDataStruct->statusInfo.bits.msgDecrypted == 1)
    console->print(F("Not decrypted"));
  else if (ubxDataStruct->statusInfo.bits.msgDecrypted == 2)
    console->print(F("Successfully decrypted"));
  else
    console->print(F("Unknown"));

  console->println();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool initGNSS(void)
{
  // Start I2C. Connect to the GNSS.

  I2C_1.begin((int)SDA_1, (int)SCL_1); // Start I2C

  // myGNSS.enableDebugging(*console); // Uncomment this line to show debug messages on the console

  if (myGNSS.begin(I2C_1) == false) // Connect to the Ublox module using Wire port
  {
    console->println(F("u-blox GNSS not detected at default I2C address!"));
    return false;
  }

  console->println(F("u-blox GNSS module connected"));

  myGNSS.setI2COutput(COM_TYPE_UBX);                  // Set the I2C port to output NMEA messages only
  myGNSS.setI2CInput(COM_TYPE_UBX | COM_TYPE_SPARTN); // Be sure SPARTN input is enabled

  myGNSS.setDGNSSConfiguration(SFE_UBLOX_DGNSS_MODE_FIXED); // Set the differential mode - ambiguities are fixed whenever possible

  myGNSS.setVal8(UBLOX_CFG_SPARTN_USE_SOURCE, 0); // Use IP source (default). Change this to 1 for L-Band (PMP)

  myGNSS.setNavigationFrequency(1); // Set output in Hz.

  myGNSS.setVal8(UBLOX_CFG_MSGOUT_UBX_RXM_COR_I2C, 1); // Enable UBX-RXM-COR messages on I2C

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Setup callbacks

  myGNSS.setAutoPVTcallbackPtr(&printPVTdata); // Enable automatic NAV PVT messages with callback to printPVTdata so we can watch the carrier solution go to fixed

  myGNSS.setRXMCORcallbackPtr(&printRXMCOR); // Print the contents of UBX-RXM-COR messages so we can check if the SPARTN data is being decrypted successfully

  return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
