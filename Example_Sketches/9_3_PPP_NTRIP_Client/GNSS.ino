//----------------------------------------
// GNSS initialization
void gnssSetup(TwoWire &i2c)
{
    // Uncomment the next line to enable the 'major' GNSS debug messages on Serial so you can see what AssistNow data is being sent
    //myGNSS.enableDebugging(Serial, true);

    if (myGNSS.begin(i2c) == false) //Connect to the Ublox module using Wire port
    {
        Serial.println(F("u-blox GPS not detected at default I2C address. Please check wiring. Freezing."));
        while (1)
        ;
    }
    Serial.println(F("u-blox module connected"));

    myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA);                 //Set the I2C port to output both NMEA and UBX messages
    myGNSS.setI2CInput(COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3); //Be sure RTCM3 input is enabled. UBX + RTCM3 is not a valid state.

    myGNSS.setDGNSSConfiguration(SFE_UBLOX_DGNSS_MODE_FIXED); // Set the differential mode - ambiguities are fixed whenever possible

    myGNSS.setNavigationFrequency(1); //Set output in Hz.

    // Set the Main Talker ID to "GP". The NMEA GGA messages will be GPGGA instead of GNGGA
    myGNSS.setMainTalkerID(SFE_UBLOX_MAIN_TALKER_ID_GP);

    myGNSS.newCfgValset(VAL_LAYER_RAM_BBR); // Use cfgValset to disable / enable individual NMEA messages
    myGNSS.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GLL_I2C, 0); // Several of these are on by default so let's disable them
    myGNSS.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GSA_I2C, 0);
    myGNSS.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GSV_I2C, 0);
    myGNSS.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_RMC_I2C, 0);
    myGNSS.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_VTG_I2C, 0);
    myGNSS.addCfgValset(UBLOX_CFG_MSGOUT_NMEA_ID_GGA_I2C, 10); // Output GGA once every 10 seconds
    if (myGNSS.sendCfgValset()) // Send the configuration VALSET
        Serial.println(F("NMEA messages were configured successfully"));
    else
        Serial.println(F("NMEA message configuration failed!"));

    // Increase transactions to reduce transfer time
    myGNSS.i2cTransactionSize = 128;

    // Auto-send Valset messages before the buffer is completely full
    myGNSS.autoSendCfgValsetAtSpaceRemaining(16);

    // Initialize the callbacks
    myGNSS.setAutoPVTcallbackPtr(&gnssStorePVTdata); // Enable automatic NAV PVT messages with callback to storePVTdata
    myGNSS.setAutoHPPOSLLHcallbackPtr(
        &gnssStoreHPdata); // Enable automatic NAV HPPOSLLH messages with callback to storeHPdata

    gnssOnline = true;
}

//----------------------------------------
void gnssStoreHPdata(UBX_NAV_HPPOSLLH_data_t *ubxDataStruct)
{
    gnssHorizontalAccuracy = ((float)ubxDataStruct->hAcc) / 10000.0; // Convert hAcc from mm*0.1 to m

    gnssLatitude = ((double)ubxDataStruct->lat) / 10000000.0;
    gnssLatitude += ((double)ubxDataStruct->latHp) / 1000000000.0;
    gnssLongitude = ((double)ubxDataStruct->lon) / 10000000.0;
    gnssLongitude += ((double)ubxDataStruct->lonHp) / 1000000000.0;
}

//----------------------------------------
void gnssStorePVTdata(UBX_NAV_PVT_data_t *ubxDataStruct)
{
    gnssAltitude = ubxDataStruct->height / 1000.0;

    gnssDay = ubxDataStruct->day;
    gnssMonth = ubxDataStruct->month;
    gnssYear = ubxDataStruct->year;

    gnssHour = ubxDataStruct->hour;
    gnssMinute = ubxDataStruct->min;
    gnssSecond = ubxDataStruct->sec;
    gnssNanosecond = ubxDataStruct->nano;
    gnssMillisecond = ceil((ubxDataStruct->iTOW % 1000) / 10.0); // Limit to first two digits

    gnssSatellitesInView = ubxDataStruct->numSV;
    gnssFixType = ubxDataStruct->fixType; // 0 = no fix, 1 = dead reckoning only, 2 = 2D-fix, 3 = 3D-fix, 4 = GNSS + dead
                                         // reckoning combined, 5 = time only fix
    gnssCarrierSolution = ubxDataStruct->flags.bits.carrSoln;

    gnssValidDate = ubxDataStruct->valid.bits.validDate;
    gnssValidTime = ubxDataStruct->valid.bits.validTime;
    gnssConfirmedDate = ubxDataStruct->flags2.bits.confirmedDate;
    gnssConfirmedTime = ubxDataStruct->flags2.bits.confirmedTime;
    gnssFullyResolved = ubxDataStruct->valid.bits.fullyResolved;
    gnssTAcc = ubxDataStruct->tAcc; // Nanoseconds
}

//----------------------------------------
// Display the location
void gnssDisplayLocation(uint32_t mSecInterval)
{
    double altitudeFeet;
    int correction;
    double hpaInches;
    char hpaUnits;
    static uint32_t lastDisplayMsec;
    int tzHours;
    int tzMinutes;
    int tzSeconds;

    // Display the horizontal accuracy
    if (gnssOnline && ((millis() - lastDisplayMsec) >= mSecInterval))
    {
        lastDisplayMsec = millis();

        // Adjust for the timezone
        correction = gnssSecond + TIME_ZONE_SECONDS;
        tzSeconds = (correction + 60) % 60;
        tzMinutes = correction / 60;
        correction = gnssMinute + tzMinutes + TIME_ZONE_MINUTES;
        tzMinutes = (correction + 60) % 60;
        tzHours = correction / 60;
        correction = gnssHour + tzHours + TIME_ZONE_HOURS;
        tzHours = (correction + 24) % 24;
        if (gnssClientUnitsFeetInches)
        {
            // Convert the altitude
            altitudeFeet = gnssAltitude * 1000. / MILLIMETERS_PER_FOOT;

            // Convert the horizontal position accuracy
            hpaUnits = '"';
            hpaInches = gnssHorizontalAccuracy * 1000. / MILLIMETERS_PER_INCH;
            if (hpaInches >= 10.)
            {
                hpaUnits = 'm';
                hpaInches = gnssHorizontalAccuracy;
            }
            Serial.printf("%2d:%02d:%02d  SIV: %2d, HPA: %.3f%c, Lat: %14.9f, Long: %14.9f, Alt: %9.3f', FT: %d, CS: %d\r\n",
                          tzHours, tzMinutes, tzSeconds,
                          gnssSatellitesInView, hpaInches, hpaUnits,
                          gnssLatitude, gnssLongitude, altitudeFeet,
                          gnssFixType, gnssCarrierSolution);
        }
        else
            Serial.printf("%2d:%02d:%02d  SIV: %2d, HPA: %.3fm, Lat: %14.9f, Long: %14.9f, Alt: %9.3fm, FT: %d, CS: %d\r\n",
                          tzHours, tzMinutes, tzSeconds,
                          gnssSatellitesInView, gnssHorizontalAccuracy,
                          gnssLatitude, gnssLongitude, gnssAltitude,
                          gnssFixType, gnssCarrierSolution);
    }
}

//----------------------------------------
// Query GNSS for current leap seconds
uint8_t gnssGetLeapSeconds()
{
    uint8_t leapSeconds;

    // Check to see if we've already set it
    if (gnssOnline)
    {
        sfe_ublox_ls_src_e leapSecSource;
        leapSeconds = myGNSS.getCurrentLeapSeconds(leapSecSource);
        return leapSeconds;
    }
    return 18; // Default to 18 if GNSS is offline
}

//----------------------------------------
void gnssUpdate()
{
    if (gnssOnline)
    {
        myGNSS.checkUblox();     // Regularly poll to get latest data and any RTCM
        myGNSS.checkCallbacks(); // Process any callbacks: ie, eventTriggerReceived
    }
}
