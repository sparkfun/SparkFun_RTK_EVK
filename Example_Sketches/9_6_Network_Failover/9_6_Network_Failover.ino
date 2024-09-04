#include <ETH.h>
#include <PPP.h>
#include <SPI.h>
#include <WiFi.h>
//#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <Wire.h>

#include "secrets.h"
#include "base64.h" //Built-in. Needed for NTRIP Client credential encoding.

#define NETWORK_DEBUG_STATE     true
#define NETWORK_DEBUG_SEQUENCE  true

//----------------------------------------

#define TIME_ZONE_HOURS         -10
#define TIME_ZONE_MINUTES       0
#define TIME_ZONE_SECONDS       0

#define DELAY_SEC(s)            (s * 1000)
#define DELAY_MIN(m)            (60 * DELAY_SEC(m))
#define DELAY_HR(h)             (60 * DELAY_MIN(h))

//----------------------------------------

bool debugCorrections = false;
bool debugNtripClientRtcm = false;
bool debugNtripClientState = true;
const char * platformPrefix = "EVK";

//----------------------------------------

#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
SFE_UBLOX_GNSS myGNSS;
bool gnssOnline;
bool gnssClientUnitsFeetInches = true;

float gnssHorizontalAccuracy;
double gnssLatitude;
double gnssLongitude;

float gnssAltitude;

uint8_t gnssDay;
uint8_t gnssMonth;
uint16_t gnssYear;
uint8_t gnssHour;
uint8_t gnssMinute;
uint8_t gnssSecond;
int32_t gnssNanosecond;
uint16_t gnssMillisecond; // Limited to first two digits

uint8_t gnssSatellitesInView;
uint8_t gnssFixType;
uint8_t gnssCarrierSolution;

bool gnssValidDate;
bool gnssValidTime;
bool gnssConfirmedDate;
bool gnssConfirmedTime;
bool gnssFullyResolved;
uint32_t gnssTAcc;

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

#define MILLIMETERS_PER_INCH    25.4
#define MILLIMETERS_PER_FOOT    (MILLIMETERS_PER_INCH * 12.)

//----------------------------------------

const int SDA_1 = 21; // ZED-F9P and NEO-D9S
const int SCL_1 = 22; // ZED-F9P and NEO-D9S

TwoWire I2C_1 = TwoWire(0);

WiFiMulti wifiMulti;

const int ETHERNET_CS = 27;  // Chip select for the WizNet W5500
const int PWREN = 32;        // 74HC4066 switch Enable - pull high to enable SCL2/SDA2 and GNSS_INT
const int ETHERNET_INT = 39; // WizNet W5500 interrupt

#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 0
#define ETH_PHY_CS ETHERNET_CS
#define ETH_PHY_IRQ ETHERNET_INT
#define ETH_PHY_RST -1

#define ETH_SPI_SCK 18
#define ETH_SPI_MISO 19
#define ETH_SPI_MOSI 23

//----------------------------------------
// RTK EVK hardware pins

// Power control
#define PWREN               32

// LARA pins
#define CELLULAR_CTS        -1
#define CELLULAR_RTS        -1
#define CELLULAR_RX         14
#define CELLULAR_TX         13
#define LARA_PWR            26

#define CELLULAR_MODEM_RST_LOW  false  //active HIGH

#define CELLULAR_RST        26    // Using the power pin as reset

// LARA R6001D no flow control
#define CELLULAR_MODEM_FC       ESP_MODEM_FLOW_CONTROL_NONE

//----------------------------------------

#define NTRIP_CLIENT_PUSHING_RTCM       ntripClientIsPushingRtcm()

//----------------------------------------

typedef void (* NETWORK_POLL_ROUTINE)(uint8_t priority, uintptr_t parameter, bool debug);

typedef struct _NETWORK_POLL_SEQUENCE
{
    NETWORK_POLL_ROUTINE routine; // Address of poll routine, nullptr at end of table
    uintptr_t parameter;          // Parameter passed to poll routine
    const char * description;     // Description of operation
} NETWORK_POLL_SEQUENCE;

extern NETWORK_POLL_SEQUENCE laraBootSequence[];
extern NETWORK_POLL_SEQUENCE laraOnSequence[];
extern NETWORK_POLL_SEQUENCE laraOffSequence[];

// Poll routines
void cellularAttached(uint8_t priority, uintptr_t parameter, bool debug);
void cellularSetup(uint8_t priority, uintptr_t parameter, bool debug);
void cellularStart(uint8_t priority, uintptr_t parameter, bool debug);
void networkDelay(uint8_t priority, uintptr_t parameter, bool debug);
void networkStartDelayed(uint8_t priority, uintptr_t parameter, bool debug);

//----------------------------------------

// Network priority entry
typedef struct _NETWORK_PRIORITY
{
    NetworkInterface * netif;       // Network interface object address
    const char * name;              // Name of the network interface
    NETWORK_POLL_SEQUENCE * boot;   // Boot sequence, may be nullptr
    NETWORK_POLL_SEQUENCE * start;  // Start sequence (Off --> On), may be nullptr
    NETWORK_POLL_SEQUENCE * stop;   // Stop routine (On --> Off), may be nullptr
} NETWORK_PRIORITY;

typedef uint8_t NetMask_t;      // One bit for each network interface

// Network priority listed in decending order
// Multiple networks may running in parallel with highest priority being
// set to the default network.  The start routine is called as the priority
// drops to that level.  The stop routine is called as the priority rises
// above that level.  The priority will continue to fall or rise until a
// network is found that is online.
const NETWORK_PRIORITY networkPriorityTable[] =
{ // Interface  Name            Boot Sequence           Start Sequence      Stop Sequence
    {&ETH,      "Ethernet",     nullptr,                nullptr,            nullptr},
    {&PPP,      "Cellular",     laraBootSequence,       laraOnSequence,     laraOffSequence},
    {&WiFi.STA, "WiFi",         nullptr,                nullptr,            nullptr},
};
const int networkPriorityTableEntries = sizeof(networkPriorityTable) / sizeof(networkPriorityTable[0]);

#define NETWORK_OFFLINE     networkPriorityTableEntries

// Network priorities
NetMask_t ethernetPriority = NETWORK_OFFLINE;
NetMask_t cellularPriority = NETWORK_OFFLINE;
NetMask_t wifiPriority = NETWORK_OFFLINE;

// Network support routines
bool networkIsInterfaceOnline(uint8_t priority);
void networkMarkOffline(int priority);
void networkMarkOnline(int priority);
uint8_t networkPriorityGet(NetworkInterface *netif);
void networkPriorityValidation(uint8_t priority);
void networkSequenceBoot(uint8_t priority);
void networkSequenceNextEntry(uint8_t priority);

//----------------------------------------
// System initialization
void setup()
{
    delay(1000);

    Serial.begin(115200);
    Serial.println();

    Serial.println("SparkFun EVK Example");

    // Listen for modem events
    Network.onEvent(networkOnEvent);

    // Set LARA_PWR low
    // LARA_PWR is inverted by the RTK EVK level-shifter
    // High 'pushes' the LARA PWR_ON pin, toggling the power
    // Configure the pin here as PPP doesn't configure _pin_rst until .begin is called
    pinMode(LARA_PWR, OUTPUT);
    digitalWrite(LARA_PWR, LOW);

    // Now enable the 3.3V regulators for the GNSS and LARA
    pinMode(PWREN, OUTPUT);
    digitalWrite(PWREN, HIGH);

    // Start I2C. Connect to the GNSS.
    I2C_1.begin((int)SDA_1, (int)SCL_1); //Start I2C

    delay(2000); // Wait for the power to stabilize

    // Initialize the network interfaces
    ethernetSetup();
    wifiSetup();
    laraSetup();

    // Start the GNSS device
    gnssSetup(I2C_1);
}

//----------------------------------------
// Main loop
void loop()
{
    networkPoll();
    ntripClientUpdate();
    gnssUpdate();
    gnssDisplayLocation(5 * 1000);
}
