


The SparkFun RTK EVK (Evaluation Kit) is the perfect development platform for your fixed or mobile high-precision positioning and navigation needs. In terms of connectivity, the SparkFun RTK EVK has the lot: WiFi / BT, Ethernet and LTE cellular! We called it the EVK (Evaluation Kit) as it truly covers all the options: L1 + L2 RTK GNSS, with L-Band correction built-in if needed, running on an agile processor with memory to spare!

<center>
<div class="grid cards" style="width:500px;" markdown>

-   <a href="https://www.sparkfun.com/products/24342">
      <figure markdown>
        <img src="../assets/img/24342-RTK-EVK-RTK_Fixed_Solution.jpg" style="width:140px; height:140px; object-fit:contain;" alt="SparkFun RTK EVK">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/24342">
      <b>SparkFun RTK EVK</b>
      <br />
      GPS-24342
      <br />
      <center>[Purchase from SparkFun :fontawesome-solid-cart-plus:](https://www.sparkfun.com/products/24342){ .md-button .md-button--primary }</center>
    </a>
</div>
</center>

!!! note
    Currently, the RTK Everywhere firmware does not support cellular with the LARA-R6. However, we have written and tested extra code examples which will allow you to stream MQTT correction data from a u-blox PointPerfect Localized Distribution topic via the LARA-R6 cellular chip. This example demonstrates how easy it is to subscribe to PointPerfect and only receive the IP correction data you need for your location, minimizing your cellular data costs.

In this tutorial we'll go over the hardware, assembly, and how get started with the SparkFun RTK.



### Kit Contents

The SparkFun RTK EVK comes with everything you need to get up and running. Our kit includes:

* 1x Cased GNSS Receiver
* 1x L1/L2/L5 GNSS Surveying Antenna
* 1x Reinforced RG58 TNC-SMA Cable (10m)
* 1x RPSMA WiFi / Bluetooth Antenna
* 2x SMA LTE Cellular Antennas
* 1x 32GB microSD Card (Class 10)
* 1x USB-C Power Supply (5V, 1A wall charger)
* 1x USB-C Cable (A to C, 2m)
* 1x Ethernet Cable (CAT-6, 1m)
* 1x Hologram eUICC nano-SIM card

<div style="text-align: center;">
  <table>
    <tr style="vertical-align:middle;">
     <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><a href="../assets/img/24342-RTK-EVK-Kit-Contents_Cropped.jpg"><img src="../assets/img/24342-RTK-EVK-Kit-Contents_Cropped.jpg" width="600px" height="600px" alt="SparkFun RTK EVK and Included Parts"></a></td>
    </tr>
    <tr>
      <td style="text-align: center; vertical-align: middle; border: solid 1px #cccccc;"><i>What's in the Box?</i>
      </td>
    </tr>
  </table>
</div>



#### GNSS Mounting Accessories

Depending on where you are installing the SparkFun RTK EVK, you also might need some mounting accessories to mount the GNSS antenna. You will want to mount the antenna in a location with the best view to of the sky (i.e. a roof).

<div class="grid cards col-4" markdown>

<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/21257">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/2/1/0/2/7/SparkFun-GNSS-Antenna-Magnetic-Mount-21257-1.jpg" style="width:140px; height:140px; object-fit:contain;" alt="GNSS Magnetic Antenna Mount - 5/8" 11-TPI">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/21257">
      <b>GNSS Magnetic Antenna Mount - 5/8" 11-TPI</b>
      <br />
      GPS-21257
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/22197">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/2/2/0/9/7/22197-_01.jpg" style="width:140px; height:140px; object-fit:contain;" alt="GNSS Antenna Mounting Hardware Kit">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/22197">
      <b>GNSS Antenna Mounting Hardware Kit</b>
      <br />
      KIT-22197
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
</div>

The RTK EVK comes with a custom extruded aluminum enclosure. The slotted flanges make it easy to install and secure the enclosure in many locations. If you are going to permanently install it up on a roof, you're going to need a suitable weatherproof box too. We found that the [Orbit 57095](https://www.orbitonline.com/products/gray-outdoor-timer-cabinet) - also [available from Amazon](https://www.amazon.com/Orbit-57095-Weather-Resistant-Outdoor-Mounted-Controller/dp/B000VYGMF2) - was a good one back when we put together our first [DIY GNSS Reference Station](https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station#mini-computer-setup-option-1).

<div class="grid cards" style="width:500px; margin: 0 auto;" markdown>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
-   <a href="https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/r/600-600/assets/learn_tutorials/1/3/6/3/Roof_Enclosure.jpg" style="width:264px; height:148px; object-fit:contain;" alt="How to Build a DIY GNSS Reference Station">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station">
      <b>How to Build a DIY GNSS Reference Station</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
</div>

!!! note "AC Not Required!"
    The Orbit enclosure comes with a built-in power outlet, but you don't actually need it! The EVK can be powered by Power-over-Ethernet (PoE), meaning all you really need to run up to the roof is a standard 8-core CAT-6 Ethernet cable. Choose a PoE Ethernet Switch that meets your needs. We have had good experiences with the [TP-Link TL-SG1005P](https://www.tp-link.com/us/business-networking/poe-switch/tl-sg1005p/) - available from many retailers including [Amazon](https://www.amazon.com/TP-Link-Compliant-Shielded-Optimization-TL-SG1005P/dp/B076HZFY3F).



#### GNSS Interface Cable

The following interface cables are great if say you are mounting the SparkFun RTK EVK in your home on a desk or lab bench and installing the antenna outdoors.

<div class="grid cards col-4" markdown>

<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/21281">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/2/1/0/6/5/21281-_CAB-_01.jpg" style="width:140px; height:140px; object-fit:contain;" alt="Interface Cable - SMA Female to SMA Male (10m, RG58)">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/21281">
      <b>Interface Cable - SMA Female to SMA Male (10m, RG58)</b>
      <br />
      CAB-21281
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/22038">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/2/1/9/0/5/22038-_CAB-_01.jpg" style="width:140px; height:140px; object-fit:contain;" alt="Interface Cable - RP-SMA Male to RP-SMA Female (10M, RG58)">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/22038">
      <b>Interface Cable - RP-SMA Male to RP-SMA Female (10M, RG58)</b>
      <br />
      CAB-22038
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
</div>



#### Memory Cards

Finally, if you're going to be logging a lot of data, you might want to stock up on 32GB microSD cards too. The RTK EVK can log 'raw' GNSS data messages (RAWX and SFRBX) at 4Hz if desired. At that rate, you're logging about 10kB per second, close to 40MB per hour or 1GB per day!

<div class="grid cards" style="width:500px; margin: 0 auto;" markdown>

<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/19041">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/1/8/6/5/3/19041-microSD_Card_-_32GB__Class_10_-02.jpg" style="width:140px; height:140px; object-fit:contain;" alt="microSD Card - 32GB (Class 10)">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/19041">
      <b>microSD Card - 32GB (Class 10)</b>
      <br />
      COM-19041
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
</div>


### Tools

Below are a few tools and accessories that you may need when connecting to the I/O terminal block on the back of the RTK EVK.

<div class="grid cards col-4" markdown>

<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/25568">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/2/6/1/2/3/TOL-25568-Moray-Driver-Kit-Feature-No-Lid.jpg" style="width:140px; height:140px; object-fit:contain;" alt="iFixit Moray Driver Kit">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/25568">
      <b>iFixit Moray Driver Kit</b>
      <br />
      TOL-25568
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/12630">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/9/3/1/2/12630-Hakko-Wire-Strippers-30AWG-Feature.jpg" style="width:140px; height:140px; object-fit:contain;" alt="Wire Strippers - 30AWG (Hakko)">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/12630">
      <b>Wire Strippers - 30AWG (Hakko)</b>
      <br />
      TOL-12630
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
-   <a href="https://www.sparkfun.com/products/11367">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/parts/7/1/0/8/11367-Hook-Up_Wire_-_Assortment__Solid_Core__22_AWG_-01.jpg" style="width:140px; height:140px; object-fit:contain;" alt="Hook-Up Wire - Assortment (Solid Core, 22 AWG)">
      </figure>
    </a>

    ---

    <a href="https://www.sparkfun.com/products/11367">
      <b>Hook-Up Wire - Assortment (Solid Core, 22 AWG)</b>
      <br />
      PRT-11367
    </a>
<!-- ----------WHITE SPACE BETWEEN PRODUCTS---------- -->
</div>



### Suggested Reading

If you aren’t familiar with the following concepts, we also recommend checking out a few of these tutorials before continuing.


<div class="grid cards col-4" markdown>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
-   <a href="https://learn.sparkfun.com/tutorials/what-is-gps-rtk">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/learn_tutorials/8/1/3/Location-Wandering-GPS-combined.jpg" style="width:264px; height:148px; object-fit:contain;" alt="What is GPS RTK?">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/what-is-gps-rtk">
      <b>What is GPS RTK?</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
-   <a href="https://learn.sparkfun.com/tutorials/getting-started-with-u-center-for-u-blox">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/learn_tutorials/8/1/5/u-center.jpg" style="width:264px; height:148px; object-fit:contain;" alt="Getting Started with U-Center for u-blox">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/getting-started-with-u-center-for-u-blox">
      <b>Getting Started with U-Center for u-blox</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
-   <a href="https://learn.sparkfun.com/tutorials/gps-rtk2-hookup-guide">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/learn_tutorials/8/5/6/GPS-RTK2_GPS_RTK_SMA_ZED-F9P__Qwiic.gif" style="width:264px; height:148px; object-fit:contain;" alt="GPS-RTK2 Hookup Guide">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/gps-rtk2-hookup-guide">
      <b>GPS-RTK2 Hookup Guide</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
-   <a href="https://learn.sparkfun.com/tutorials/setting-up-a-rover-base-rtk-system">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/learn_tutorials/1/3/6/2/GNSS_RTK_DIY_Surveying_Tutorial.jpg" style="width:264px; height:148px; object-fit:contain;" alt="Setting up a Rover Base RTK System">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/setting-up-a-rover-base-rtk-system">
      <b>Setting up a Rover Base RTK System</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
-   <a href="https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station">
      <figure markdown>
        <img src="https://cdn.sparkfun.com/assets/learn_tutorials/1/3/6/3/Roof_Enclosure.jpg" style="width:264px; height:148px; object-fit:contain;" alt="How to Build a DIY GNSS Reference Station">
      </figure>
    </a>

    ---

    <a href="https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station">
      <b>How to Build a DIY GNSS Reference Station</b>
    </a>
<!-- ----------WHITE SPACE BETWEEN GRID CARDS---------- -->
</div>



You may also be interested in the following blog posts on GNSS technologies.

<div class="grid cards col-4" markdown align="center">

-   <a href="https://www.sparkfun.com/news/4276">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/4/2/7/6/GPSvGNSSHomepageImage4.png)
	</figure>

	---

	**GPS vs GNSS**</a>

-   <a href="https://www.sparkfun.com/news/7138">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/7/1/3/8/SparkFun_RTK_Facet_-_Surveying_Monopod.jpg)
	</figure>

	---

	**What is Correction Data?**</a>

-   <a href="https://www.sparkfun.com/news/7533">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/7/5/3/3/rtk-blog-thumb.png)
	</figure>

	---

	**Real-Time Kinematics Explained**</a>

-   <a href="https://www.sparkfun.com/news/7401">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/7/4/0/1/Screen_Shot_2023-06-26_at_8.30.22_PM.png)
	</figure>

	---

	**New Video: Unlocking High-Precision RTK Positioning**</a>

</div>
