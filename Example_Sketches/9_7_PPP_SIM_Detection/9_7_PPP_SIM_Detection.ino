#include <PPP.h>

//----------------------------------------

// Power control
#define PWREN             32

//----------------------------------------
// System initialization
void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    // Enable the 3.3V regulators for the GNSS and LARA
    pinMode(PWREN, OUTPUT);
    digitalWrite(PWREN, HIGH);

    // Give the LARA time to power up
    delay(1000);

    // Listen for cellular modem events
    Network.onEvent(cellularEvent);
}

//----------------------------------------
// Main loop
void loop()
{
    cellularUpdate(millis());
}
