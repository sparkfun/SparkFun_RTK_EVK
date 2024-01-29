# mazg.ch HPG Solution for the SparkFun RTK Everywhere L-Band Cellular

This example is a copy of Michael Ammann (@mazgch)'s excellent [HPG Solution](https://github.com/mazgch/hpg).

Michael designed his own [hardware](https://github.com/mazgch/hpg#quick-reference-card) based on the u-blox:
* NINA-W106
* LARA-R6001D (Cellular)
* ZED-F9R (Dead Reckoning, built-in IMU)
* NEO-D9S (L-Band corrections from PointPerfect)

The SparkFun RTK Everywhere hardware is very similar. We use:
* Espressif ESP32-WROVER (16MB Flash, 8MB PSRAM)
* LARA-R6001D (Cellular)
* ZED-F9P (RTK Base capable)
* NEO-D9S (L-Band corrections from PointPerfect)

### Use

* Compile and upload the code
  * Select the "ESP32 Wrover Module" and the "Huge APP" partition scheme
* Connect your computer to the ESP32 WiFi access point (hot spot)
  * It is a captive portal and opens a web page automatically
  * If the HPG Solution web page does not appear, try opening 192.168.4.1 in Edge. It does not seem to work using Chrome
* Open the WiFi Config tab
* Enter the SSID and Password of your WiFi network
* Save and restart the HPG Solution if necessary
* The ESP32 will connect to your WiFi network
* Navigate to its IP address to view the HPG Solution web page
  * The IP address is shown in the Serial Console
