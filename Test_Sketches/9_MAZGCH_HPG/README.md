# mazg.ch HPG Solution for the SparkFun RTK Everywhere L-Band Cellular

This example is a tweaked copy of Michael Ammann (@mazgch)'s excellent [HPG Solution](https://github.com/mazgch/hpg).

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
* Connect your computer to the ESP32 WiFi
  * It is a captive portal and opens a web page automatically

### Tweaks

We tweaked Michael's software only a little to add the pin definitions for the RTK Everywhere.
See [HW.h](./software/HW.h). The definitions for the SPARKFUN_RTK_CONTROL start at about line 194.

