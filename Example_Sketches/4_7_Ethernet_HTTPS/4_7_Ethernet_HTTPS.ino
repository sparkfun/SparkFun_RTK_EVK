#include <SPI.h>
#include <ETH.h>
#include <Network.h>
#include <NetworkClientSecure.h>

#include "Cert_AWS.h"
#include "Cert_Google.h"

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

bool RTK_CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC = false;

//----------------------------------------

typedef bool (* IS_CONNECTED)();

//----------------------------------------

typedef struct _HTTPS_CLIENT_CONNECTION
{
    IS_CONNECTED isInternetAvailable;
    const char * rootCA;
    const char * hostName;
    const char * url;

    // The following parameters are initialized to zero (false)
    bool suppressFirstPageOutput;
    NetworkClientSecure client;
    int headerLength;
    int pageLength;
    uint8_t hccState;
    bool tagEndFound;
    int tagEndOffset;
    bool tagStartFound;
    int tagStartOffset;
    uint32_t timer;
    uint8_t buffer[2048];
} HTTPS_CLIENT_CONNECTION;

HTTPS_CLIENT_CONNECTION google;
HTTPS_CLIENT_CONNECTION SparkFun;

//----------------------------------------
void setup()
{
    pinMode(ETHERNET_CS, OUTPUT);
    digitalWrite(ETHERNET_CS, HIGH);
    pinMode(PWREN, OUTPUT);
    digitalWrite(PWREN, HIGH);

    delay(1000);

    Serial.begin(115200);
    Serial.println();
    Serial.println("SparkFun RTK EVK - Test Sketch");

    // Initialize SPI
    SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI);

    // Initialize the network
    Network.onEvent(networkOnEvent);

    // Initialize Ethernet
    ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI);

    // Set the HTTPS parameters
    google.rootCA = caGoogle;
    google.hostName = "www.google.com";
    google.url = "/";
    google.isInternetAvailable = ethernetIsInternetAvailable;
    google.suppressFirstPageOutput = true;

    SparkFun.rootCA = caAmazonWebServices;
    SparkFun.hostName = "www.sparkfun.com";
    SparkFun.url = "/";
    SparkFun.isInternetAvailable = ethernetIsInternetAvailable;
    SparkFun.suppressFirstPageOutput = true;
}

//----------------------------------------
void loop()
{
    httpsUpdate(&google, 443, 15 * 1000);
    httpsUpdate(&SparkFun, 443, 16 * 1000);
}
