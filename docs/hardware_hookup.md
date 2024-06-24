In this section, we'll go over how to connect to the SparkFun RTK EVK.

### Cellular Antenna

To take advantage of the LTE cellular network with the LARA-R6, you will need to connect to the two LTE antennas on the back panel. Insert the LTE Hinged External Antenna into the SMA connector labeled as **Cell 1** and rotate the threaded connector until it is finger tight. Repeat for the other SMA connector labeled as **Cell 2**. You will need to adjust the antenna as necessary depending on where you mount the RTK EVK.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_Cellular_Antenna_Connectors.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_Cellular_Antenna_Connectors.jpg" width="600px" height="600px" alt="LTE Cellular Antennas Ports Highlighted"></a></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/16432-698MHz-2.7GHz_LTE_Hinged_External_Antenna__with_SMA_Male_Connector-01.jpg"><img src="../assets/img/16432-698MHz-2.7GHz_LTE_Hinged_External_Antenna__with_SMA_Male_Connector-01.jpg" width="600px" height="600px" alt="LTE Cellular Antennas Connected"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>LTE Cellular Antenna</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>LTE Cellular Antenna Ports Highlighted</i></td>
    </tr>
  </table>
</div>



### Multi-Band GNSS Antenna

Of course, you will need to connect an active multi-band GNSS antenna to the RTK EVK. We recommend using the L1/L2/L5 Surveying Antenna (SPK6618H) that is included with the kit. This covers the L1 and L2 band for the ZED-F9P. This also includes a built-in ground plane to provide the best performance for GNSS. This antenna will also provide broad coverage of the L-Band for the correction signals. Insert the SMA connector from the interface cable to the SMA connector labeled as **GNSS**. Tighten the hex nut until it is finger tight. Connect the BNC connector side from the interface cable to the to the SPK6618H antenna. Tighten the threaded connector until it is finger tight.




<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_GNSS_Antenna_Connector.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_GNSS_Antenna_Connector.jpg" width="600px" height="600px" alt="Cable and Survey Antenna Connected"></a></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/SparkFun_Reinforced_Interface_Cable_-_SMA_Male_to_TNC_Male_-_1.jpg"><img src="../assets/img/SparkFun_Reinforced_Interface_Cable_-_SMA_Male_to_TNC_Male_-_1.jpg" width="600px" height="600px" alt="Cable"></a></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/SparkFun_GNSS_SPK6618H_Triband_Antenna_-_2.jpg"><img src="../assets/img/SparkFun_GNSS_SPK6618H_Triband_Antenna_-_2.jpg" width="600px" height="600px" alt="Survey Antenna"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>GNSS Antenna Port Highlighted</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Cable</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Survey Antenna</i></td>
    </tr>
  </table>
</div>

You will need to mount the antenna in an area where it gest the best view of the sky (we recommend the roof). There is mounting hardware where you can permanently mount the antenna to a structure or support. There is also a magnetic mount to attach to a metal structure as well. For the scope of this tutorial, we will not go over the details of installing the antenna.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/assembly-gnss-mount_location.jpg"><img src="../assets/img/assembly-gnss-mount_location.jpg" width="600px" height="600px" alt="Cable and Survey Antenna Connected"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Cable Connected and Survey Antenna Mounted on Magnetic Stand</i></td>
    </tr>
  </table>
</div>



### WiFi/Bluetooth Antenna

For WiFi and Bluetooth using the ESP32, you will need to connect the 2.4GHz Duck Antenna to the RPSMA connector labeled as **ESP32**. Insert the antenna into the RPSMA connector and tighten the threaded connector until it is finger tight.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_WiFI_Antenna_Connector.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_WiFI_Antenna_Connector.jpg" width="600px" height="600px" alt="""></a></td>
    <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/00145-02-L.jpg"><img src="../assets/img/00145-02-L.jpg" width="600px" height="600px" alt="WiFi/Bluetooth Antenna"></a></td>
   </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>WiFi/Bluetooth Connector Highlighted</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>WiFi/Bluetooth Antenna</i></td>
    </tr>
  </table>
</div>

!!! note
    For users that need an extension cable to mount the antenna to a location that is farther away from the RTK EVK, you could use the [Interface Cable - RP-SMA Male to RP-SMA Female (10M, RG58)](https://www.sparkfun.com/products/22038).



### Ethernet

For users that are connecting Ethernet or using power from the PoE, you will need to connect the Ethernet cable between the RTK EVK and your PoE network switch. Insert the Ethernet cable to the RJ45 MagJack connector until you hear a click. Then insert the other end into the network switch.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_Ethernet_LEDs.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Back_Ethernet_LEDs.jpg" width="600px" height="600px" alt="Ethernet Port Highlighted"></a></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/08915-03.jpg"><img src="../assets/img/08915-03.jpg" width="600px" height="600px" alt="Ethernet Cable"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Ethernet Port Highlighted</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Ethernet Cable</i></td>
    </tr>
  </table>
</div>



### Connecting to the ESP32

You can connect to the ESP32 by inserting the USB-C connector into the port labeled as **CONFIG ESP32** and inserting the other end of the cable to your computer's USB port. This will power the RTK EVK, allow you to upload custom code, manually update the firmware binaries, or connect to a serial terminal.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_USB_ESP32.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_USB_ESP32.jpg" width="600px" height="600px" alt="USB CONFIG ESP32 Port Highlighted"></a></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/15424-Reversible_USB_A_to_C_Cable_-_2m-01.jpg"><img src="../assets/img/15424-Reversible_USB_A_to_C_Cable_-_2m-01.jpg" width="600px" height="600px" alt="USB C Cable"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>USB CONFIG ESP32 Port Highlighted</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>USB C Cable</i></td>
    </tr>
  </table>
</div>



### Connecting to the u-blox Modules

You can connect to the ZED-F9P, NEO-D9S, or LARA-R6 by inserting the USB-C connector into the port labeled as **CONFIG UBLOX** and inserting the other end to your computer's USB port.  This will power the RTK EVK and allow you to connect the modules to u-center.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_USB_ublox.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_USB_ublox.jpg" width="600px" height="600px" alt="USB CONFIG ESP32 Port Highlighted"></a></td><td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/15424-Reversible_USB_A_to_C_Cable_-_2m-01.jpg"><img src="../assets/img/15424-Reversible_USB_A_to_C_Cable_-_2m-01.jpg" width="600px" height="600px" alt="USB C Cable"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>USB CONFIG ESP32 Port Highlighted</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>USB C Cable</i></td>
    </tr>
  </table>
</div>




### Inserting a MicroSD Card

To insert a microSD card, slide the microSD card into the microSD socket until it clicks in place. Pushing the microSD card again will eject the memory card.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_MicroSD_Card_Slot.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_MicroSD_Card_Slot.jpg" width="600px" height="600px" alt="MicroSD Card Port Highlighted"></a></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/19041-microSD_Card_-_32GB__Class_10_-02.jpg"><img src="../assets/img/19041-microSD_Card_-_32GB__Class_10_-02.jpg" width="600px" height="600px" alt="MicroSD Card Port Highlighted"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>MicroSD Card Port Highlighted</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>MicroSD Card</i></td>
    </tr>
  </table>
</div>



### Inserting a Nano SIM Card

!!!note
    ####Activate Your Hologram SIM
    If you're using a SIM card from Hologram, you'll need to follow a few quick steps to activate your SIM card.
    <ol>
      <li><a href="https://dashboard.hologram.io/">Log in</a> to your Hologram account, or <a href="https://dashboard.hologram.io/account/register">create one</a>.</li>
      <li>Click the blue <strong>+ Activate SIM</strong> button in the upper-right-corner of your Dashboard.</li>
      <br>
      <p style="text-align:center;"> <a href="https://cdn.sparkfun.com/assets/learn_tutorials/8/1/6/hologram-activate-sim.png"><img src="https://cdn.sparkfun.com/r/600-600/assets/learn_tutorials/8/1/6/hologram-activate-sim.png" alt="Activate hologram SIM"></a></p>
      <br>
      <li>Select your plan – in most cases “Maker Flexible” is the way to go, but you can upgrade.</li>
      <li>Enter your SIM card’s CCID. This number can be found printed on both your nano-SIM card and in the larger digits below the bar code. Then select continue.</li>
      <li>Next you can decide whether to enable auto-refill or not and continue. Finally, you’ll be greeted with a summary page – hit “Activate” and you’re ready to go!</li>
    </ol>
    <p>For more help activating your Hologram SIM card, check out their <a  href="https://hologram.io/docs/guide/connect/connect-device/">Connect Your Device</a> documentation.</p>

To take advantage of the LARA-R6, insert the nano SIM card into the nano SIM socket until it clicks. In this case, we used a [Hologram card](https://www.sparkfun.com/products/17117).

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_Nano_SIM_Card_Slot.jpg"><img src="../assets/img/24342-RTK-EVK-High-Precision-GNSS-Front_Nano_SIM_Card_Slot.jpg" width="600px" height="600px" alt="Nano SIM Card Socket Highlighted"></a></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/17117-Hologram_eUICC_SIM_Card-01.jpg"><img src="../assets/img/17117-Hologram_eUICC_SIM_Card-01.jpg" width="600px" height="600px" alt="Nano SIM Card Socket Highlighted"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Nano SIM Card Socket Highlighted</i></td>
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Nano SIM Card</i></td>
    </tr>
  </table>
</div>


!!!note
    You will need to remove the nano SIM card from the 3-in-1 SIM card holder when you initially receive the Hologram card. Just "punch" out the pre-cut nano SIM card size out by pushing it out with your fingers. Make sure to avoid touching the gold contacts with your bare hands.



### Inserting a Wire into the I/O Screw Terminal

To connect to a ppin on the screw terminal block, insert a [stripped wire of any length](https://learn.sparkfun.com/tutorials/working-with-wire/how-to-strip-a-wire) into the I/O screw terminal. Of course, you could use jumper wires with pins as well. Secure the wire by tightening the screw using a precision flathead screw driver. Test the connection by gently pulling the wires to ensure that they are secure.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK_Screw_Terminal_Wire.jpg"><img src="../assets/img/24342-RTK-EVK_Screw_Terminal_Wire.jpg" width="600px" height="600px" alt="Wire Inserted into I/O and Screw Being Tightened"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>Wire Inserted into I/O and Screw Being Tightened</i></td>
    </tr>
  </table>
</div>

If necessary, users can remove the terminal block off by wiggling the connector or using a spudger. When ready, users can slide the I/O terminal block back into the socket.

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-Removing_IO_Screw_Terminal_Block.jpg"><img src="../assets/img/24342-RTK-EVK-Removing_IO_Screw_Terminal_Block.jpg" width="600px" height="600px" alt="I/O Screw Terminal Block Removed"></a></td>
    </tr>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>I/O Screw Terminal Block Removed</i></td>
    </tr>
  </table>
</div>


!!! tip
    Don't have spudger? A good substitute includes a flathead screw driver, plastic card, or the PCB ruler. Using a tool will make it easier to remove the I/O terminal block.



### Connecting a Radio Transceiver

Users can connect a radio transceiver to the ZED-F9P UART2 port for correction data. We recommend using a [breadboard to JST-GHR-06V cable](https://www.sparkfun.com/products/23353) to connect the SiK Telemetry Radio. Note that the cable is included with the LoRaSerial Kit.

<div class="grid cards col-4" markdown>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/19032">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/1/8/6/3/4/19032-SiK_Telemetry_Radio_V3_-_915MHz__100mW-01.jpg" style="width:140px; height:140px; object-fit:contain;" alt="SiK Telemetry Radio V3 - 915MHz, 100mW">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/19032">
      <b>SiK Telemetry Radio V3 - 915MHz, 100mW</b>
      <br />
      WRL-19032
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/20029">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/1/9/7/9/0/SparkFun_LoRaSerial_Enclosed_-_20029-1.jpg" style="width:140px; height:140px; object-fit:contain;" alt="SparkFun LoRaSerial Kit - 915MHz (Enclosed)">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/20029">
      <b>SparkFun LoRaSerial Kit - 915MHz (Enclosed)</b>
      <br />
      KIT-20029
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/23353">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/1/9/0/9/3/23353-_1.jpg" style="width:140px; height:140px; object-fit:contain;" alt="Breadboard to JST-GHR-06V Cable - 6-Pin x 1.25mm Pitch (For LoRaSerial)">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/23353">
      <b>Breadboard to JST-GHR-06V Cable - 6-Pin x 1.25mm Pitch (For LoRaSerial)</b>
      <br />
      CAB-23353
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
</div>

For users with the using a telemetry radio, you will need to connect the UART pins and ground to the screw terminal block. Note that this connection is only for the serial UART connection. To power, we recommend using an external 5V voltage source since 5V is not broken out. While there is a 3V3 pin available, the voltage will not be enough to fully power the radio module. One method is connecting a micro-B USB cable and 5V USB wall adapter to the Telemetry Radio.

<div style="text-align: center;">
    <table>
        <tr>
            <th style="text-align: center; border: solid 1px #cccccc;">RTK EVK<br />UART Pinout
            </th>
            <th style="text-align: center; border: solid 1px #cccccc;">Telemetry<br />UART Pinout
            </th>
        </tr>
        <tr>        
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#d4edda"><font color="#000000">ZED TX2</font>
            </td>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#d4edda"><font color="#000000">RX</font>
            </td>
        </tr>
        <tr>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#ffdaaf"><font color="#000000">ZED RX2</font>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#ffdaaf"><font color="#000000">TX</font>
            </td>
        </tr>
        <tr>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#DDDDDD"><font color="#000000">GND</font>
            </td>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#DDDDDD"><font color="#000000">GND</font>
            </td>
        </tr>
    </table>
</div>

For users with the LoRaSerial, you will also be connecting the UART pins and ground to the screw terminal block. To power, we also recommend using an external 5V voltage source for power such as a USB C cable and 5V USB wall adapter.

<div style="text-align: center;">
    <table>
        <tr>
            <th style="text-align: center; border: solid 1px #cccccc;">RTK EVK<br />UART Pinout
            </th>
            <th style="text-align: center; border: solid 1px #cccccc;">LoRaSerial<br />UART Pinout
            </th>
        </tr>
        <tr>        
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#d4edda"><font color="#000000">ZED TX2</font>
            </td>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#d4edda"><font color="#000000">RXI</font>
            </td>
        </tr>
        <tr>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#ffdaaf"><font color="#000000">ZED RX2</font>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#ffdaaf"><font color="#000000">TXO</font>
            </td>
        </tr>
        <tr>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#DDDDDD"><font color="#000000">GND</font>
            </td>
            <td style="text-align: center; border: solid 1px #cccccc;" bgcolor="#DDDDDD"><font color="#000000">GND</font>
            </td>
        </tr>
    </table>
</div>



### Connecting via PTH

There are various PTHs throughout the PCB. Most are used with the testbeds for quality control but they are available for you to connect! For temporary connections to the PTHs, you could use IC hooks to test out the pins. However, you'll need to solder headers or wires of your choice to the board for a secure connection. You can choose between [header pins and jumper wires](https://learn.sparkfun.com/tutorials/how-to-solder-through-hole-soldering/all), or [stripping wire and soldering the wire](https://learn.sparkfun.com/tutorials/working-with-wire/all) directly to the board.

<div class="grid cards col-4" markdown>

-   <a href="https://learn.sparkfun.com/tutorials/how-to-solder-through-hole-soldering/all">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/learn_tutorials/5/Soldering_Action-01.jpg"style="width:264px; height:148px; object-fit:contain;" alt="How to Solder: Through Hole Soldering">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/how-to-solder-through-hole-soldering/all">
      <b>How to Solder: Through Hole Soldering</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->

-   <a href="https://learn.sparkfun.com/tutorials/working-with-wire/all">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/0/5/0/0/f/5138de3cce395fbb1b000002.JPG" style="width:264px; height:148px; object-fit:contain;" alt="Working with Wire">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/working-with-wire/all">
      <b>Working with Wire</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
</div>
