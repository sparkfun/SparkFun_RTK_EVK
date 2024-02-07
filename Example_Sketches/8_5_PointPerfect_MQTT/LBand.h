#include <Wire.h>

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
SFE_UBLOX_GNSS myLBand;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool initLBand(void)
{
  // myLBand.enableDebugging(*console); // Uncomment this line to show debug messages on the console

  if (myLBand.begin(I2C_1, 0x43) == false) // Connect to the NEI-D9S using Wire port
  {
    console->println(F("u-blox L-Band not detected at default I2C address!"));
    return false;
  }

  console->println(F("u-blox L-Band module connected"));

  // Disable PMP message output on UART1 to prevent the ZED-F9P from getting correction data twice!
  myLBand.setVal8(UBLOX_CFG_MSGOUT_UBX_RXM_PMP_UART1, 0);

  return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
