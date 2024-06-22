!!! arduino
    This example assumes you are using the latest version of the Arduino IDE on your desktop. If this is your first time using the Arduino IDE, library, or board add-on, please review the following tutorials.

    - [Installing the Arduino IDE](https://learn.sparkfun.com/tutorials/installing-arduino-ide)
    - [Installing Board Definitions in the Arduino IDE](https://learn.sparkfun.com/tutorials/installing-board-definitions-in-the-arduino-ide)
    - [Installing an Arduino Library](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)

!!! note
    If you've never connected an CH340 device to your computer before, you may need to install drivers for the USB-to-serial converter. Check out our section on "[How to Install CH340 Drivers](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers)" for help with the installation.

    - [How to Install CH340 Drivers](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all)

At the time of writing this tutorial, the RTK Everywhere Firmware currently does not support the u-blox LARA-R6 module. For users that are interested in taking advantage of cellular when Ethernet or WiFi is not available, you can check out the individual Arduino Example Sketches located in the SparkFun RTK EVK **Example Sketches** folder. Besides the 6x examples for the LARA-R6, you will also notice several other examples such as using the ZED-F9P, NEO-D9S, Qwiic OLED, LEDs, antenna open/short circuit detection, Ethernet, WiFi, and datalogging using the microSD card. Head to the [SparkFun RTK EVK GitHub Repo](https://github.com/sparkfun/SparkFun_RTK_EVK/tree/main/Example_Sketches) to grab the files or download the .ZIP by clicking the button below.

<div style="text-align: center"><a href="https://github.com/sparkfun/SparkFun_RTK_EVK/archive/refs/heads/main.zip" target="rtk_evk_arduino_example_sketches" class="md-button">GitHub SparkFun RTK EVK Repo: Arduino Example Sketches</a></div>

For the scope of this tutorial, we will upload one of the examples to get started. After downloading the files, unzip the files and head to the following folder: **SparkFun_RTK_EVK-main** > **Example Sketches**. Let&apos;s test the connection between the ESP32-WROVER and LARA-R6 by clicking on the **8_1_LARA_UART_Test** > **8_1_LARA_UART_Test.ino**.

!!! note
    Some examples may include additional header files. You'll need to click on the example that has the same title as the folder's title. Certain examples may also require an additional Arduino Library. Make sure to check the example to see if there is a `#include` with the Arduino Library name near the top before compiling.  You may also need to update the examples to provide any secret keys, accounts, or passwords.

Then select the associated board definition (in this case the **ESP32 Wrover Module**) and COM port. With a USB C cable connected to the **CONFIG ESP32** port, click on the upload button. Opening the [Arduino Serial Terminal](https://learn.sparkfun.com/tutorials/terminal-basics/arduino-serial-monitor-windows-mac-linux) at **115200** baud, the output will provide the model and SIM card information. 

!!! note
   At the time of writing, we used the following:

       * Arduino IDE v2.3.2
       * esp32 by Espressif v3.0.1
