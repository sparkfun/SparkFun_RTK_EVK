/**********************************************************************
  Wifi.ino

  Handle the WiFi events
**********************************************************************/

//****************************************
// Constants
//****************************************

const bool debugEspNow = true;
const bool wifiDebug = true;
const bool wifiDisplay = true;
const bool wifiVerbose = true;

const uint8_t peerBroadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

typedef enum
{
    ESPNOW_OFF = 0,
    ESPNOW_BROADCASTING,
    ESPNOW_PAIRING,
    ESPNOW_MAC_RECEIVED,
    ESPNOW_PAIRED,
    ESPNOW_MAX
} ESP_NOW_STATE;

//****************************************
// Locals
//****************************************

ESP_NOW_STATE espNowState;
int packetRSSI;

//*********************************************************************
// Update the state of the ESP Now state machine
//
//      +---------------------+
//      |      ESPNOW_OFF     |
//      +---------------------+
//          |             |
//          |             | No pairs listed
//          |             V
//          |  +---------------------+
//          |  | ESPNOW_BROADCASTING |
//          |  +---------------------+
//          |             |
//          |             |
//          |             V
//          |  +---------------------+
//          |  |   ESPNOW_PAIRING    |
//          |  +---------------------+
//          |             |
//          |             |
//          |             V
//          |  +---------------------+
//          |  | ESPNOW_MAC_RECEIVED |
//          |  +---------------------+
//          |             |
//          |             |
//          |             V
//          |  +---------------------+
//          '->|    ESPNOW_PAIRED    |
//             +---------------------+
//
// Send RTCM in either ESPNOW_BROADCASTING or ESPNOW_PAIRED state.
// Receive RTCM in ESPNOW_BROADCASTING, ESPNOW_MAC_RECEIVED and
// ESPNOW_PAIRED states.
//*********************************************************************

//*********************************************************************
// Update the state of the ESP-NOW subsystem
/*
void espNowSetState(ESP_NOW_STATE newState)
{
    const char * name[] =
    {
        "ESPNOW_OFF",
        "ESPNOW_BROADCASTING",
        "ESPNOW_PAIRING",
        "ESPNOW_MAC_RECEIVED",
        "ESPNOW_PAIRED",
    };
    const int nameCount = sizeof(name) / sizeof(name[0]);
    const char * newName;
    char nLine[80];
    const char * oldName;
    char oLine[80];

    if (debugEspNow == true)
    {
        // Get the old state name
        if (espNowState < ESPNOW_MAX)
            oldName = name[espNowState];
        else
        {
            sprintf(oLine, "Unknown state: %d", espNowState);
            oldName = &oLine[0];
        }

        // Get the new state name
        if (espNowState < ESPNOW_MAX)
            oldName = name[espNowState];
        else
        {
            sprintf(oLine, "Unknown state: %d", espNowState);
            oldName = &oLine[0];
        }

        // Display the state change
        if (newState == espNowState)
            systemPrintf("ESP-NOW: *%s\r\n", newName);
        else
            systemPrintf("ESP-NOW: %s --> %s\r\n", oldName, newName);
    }
    espNowState = newState;
}
*/

//*********************************************************************
// Callback for all RX Packets
// Get RSSI of all incoming management packets: https://esp32.com/viewtopic.php?t=13889
void wifiPromiscuousRxHandler(void *buf, wifi_promiscuous_pkt_type_t type)
{
    const wifi_promiscuous_pkt_t *ppkt; // Defined in esp_wifi_types_native.h

    // All espnow traffic uses action frames which are a subtype of the
    // mgmnt frames so filter out everything else.
    if (type != WIFI_PKT_MGMT)
        return;

    ppkt = (wifi_promiscuous_pkt_t *)buf;
    packetRSSI = ppkt->rx_ctrl.rssi;
}

//*********************************************************************
// Callback when data is received
void espNowRxHandler(const esp_now_recv_info *mac,
                     const uint8_t *incomingData,
                     int len)
{
//    typedef struct esp_now_recv_info {
//        uint8_t * src_addr;             // Source address of ESPNOW packet
//        uint8_t * des_addr;             // Destination address of ESPNOW packet
//        wifi_pkt_rx_ctrl_t * rx_ctrl;   // Rx control info of ESPNOW packet
//    } esp_now_recv_info_t;

systemPrintf("RX %02x:%02x:%02x:%02x:%02x:%02x --> %02x:%02x:%02x:%02x:%02x:%02x, %d bytes, 0x%02x(%c)\r\n",
mac->src_addr[0], mac->src_addr[1], mac->src_addr[2],
mac->src_addr[3], mac->src_addr[4], mac->src_addr[5],
mac->des_addr[0], mac->des_addr[1], mac->des_addr[2],
mac->des_addr[3], mac->des_addr[4], mac->des_addr[5],
len, *incomingData, *incomingData);
/*
    uint8_t receivedMAC[6];

    if (espNowState == ESPNOW_PAIRING)
    {
        PairMessage pairMessage;
        if (len == sizeof(pairMessage)) // First error check
        {
            memcpy(pairMessage, incomingData, sizeof(pairMessage));

            // Check CRC
            uint8_t tempCRC = 0;
            for (int x = 0; x < 6; x++)
                tempCRC += pairMessage.macAddress[x];

            if (tempCRC == pairMessage.crc) // 2nd error check
            {
                memcpy(&receivedMAC, pairMessage.macAddress, 6);
                espNowSetState(ESPNOW_MAC_RECEIVED);
            }
            // else Pair CRC failed
        }
    }
    else
    {
        if (debugEspNow == true)
            systemPrintf("ESPNOW received %d RTCM bytes, RSSI: %d\r\n",
                         len, espnowRSSI);

        espnowRSSI = packetRSSI; // Record this packet's RSSI as an ESP NOW packet

        // We've just received ESP-Now data. We assume this is RTCM and push it directly to the GNSS.
        // Determine if ESPNOW is the correction source
        if (correctionLastSeen(CORR_ESPNOW))
        {
            // Pass RTCM bytes (presumably) from ESP NOW out ESP32-UART to GNSS
            gnss->pushRawData((uint8_t *)incomingData, len);

            if ((debugEspNow == true || settings.debugCorrections == true) && !inMainMenu)
                systemPrintf("ESPNOW received %d RTCM bytes, pushed to GNSS, RSSI: %d\r\n", len, espnowRSSI);
        }
        else
        {
            if ((debugEspNow == true || settings.debugCorrections == true) && !inMainMenu)
                systemPrintf("ESPNOW received %d RTCM bytes, NOT pushed due to priority, RSSI: %d\r\n", len,
                             espnowRSSI);
        }

        espnowIncomingRTCM = true; // Display a download icon
        lastEspnowRssiUpdate = millis();
    }
*/
}

//----------------------------------------------------------------------
// ESP-NOW bringup from example 4_9_ESP_NOW
//   1. Set station mode
//   2. Create nowSerial as new ESP_NOW_Serial_Class
//   3. nowSerial.begin
// ESP-NOW bringup from RTK
//   1. Get WiFi mode
//   2. Set WiFi station mode if necessary
//   3. Get WiFi station protocols
//   4. Set WIFI_PROTOCOL_LR protocol
//   5. Call esp_now_init
//   6. Call esp_wifi_set_promiscuous(true)
//   7. Set promiscuous receive callback [esp_wifi_set_promiscuous_rx_cb(promiscuous_rx_cb)]
//      to get RSSI of action frames
//   8. Assign a channel if necessary, call espnowSetChannel
//   9. Set receive callback [esp_now_register_recv_cb(espnowOnDataReceived)]
//  10. Add peers from settings
//      A. If no peers exist
//          i.   Determine if broadcast peer exists, call esp_now_is_peer_exist
//          ii.  Add broadcast peer if necessary, call espnowAddPeer
//          iii. Set ESP-NOW state, call espnowSetState(ESPNOW_BROADCASTING)
//      B. If peers exist,
//          i.  Set ESP-NOW state, call espnowSetState(ESPNOW_PAIRED)
//          ii. Loop through peers listed in settings, for each
//              a. Determine if peer exists, call esp_now_is_peer_exist
//              b. Add peer if necessary, call espnowAddPeer
//
// In espnowOnDataReceived
//  11. Save ESP-NOW RSSI
//  12. Set lastEspnowRssiUpdate = millis()
//  13. If in ESPNOW_PAIRING state
//      A. Validate message CRC
//      B. If valid CRC
//          i.  Save peer MAC address
//          ii. espnowSetState(ESPNOW_MAC_RECEIVED)
//  14. Else if ESPNOW_MAC_RECEIVED state
//      A. If ESP-NOW is corrections source, correctionLastSeen(CORR_ESPNOW)
//          i.  gnss->pushRawData
//  15. Set espnowIncomingRTCM
//
// ESP-NOW shutdown from RTK
//   1. esp_wifi_set_promiscuous(false)
//   2. esp_wifi_set_promiscuous_rx_cb(nullptr)
//   3. esp_now_unregister_recv_cb()
//   4. Remove all peers by calling espnowRemovePeer
//   5. Get WiFi mode
//   6. Set WiFi station mode if necessary
//   7. esp_wifi_get_protocol
//   8. Turn off long range protocol if necessary, call esp_wifi_set_protocol
//   9. Turn off ESP-NOW. call esp_now_deinit
//  10. Set ESP-NOW state, call espnowSetState(ESPNOW_OFF)
//  11. Restart WiFi if necessary
//----------------------------------------------------------------------

//*********************************************************************
// Add a peer to the ESP-NOW network
esp_err_t espNowAddPeer(const uint8_t * peerMac)
{
    esp_now_peer_info_t peerInfo;

    // Describe the peer
    memcpy(peerInfo.peer_addr, peerMac, 6);
    peerInfo.channel = 0;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.encrypt = false;

    // Add the peer
    if (debugEspNow)
        systemPrintf("Calling esp_now_add_peer\r\n");
    esp_err_t result = esp_now_add_peer(&peerInfo);
    if (result != ESP_OK)
    {
        systemPrintf("ERROR: Failed to add ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x, result: %d\r\n",
                     peerBroadcast[0], peerBroadcast[1],
                     peerBroadcast[2], peerBroadcast[3],
                     peerBroadcast[4], peerBroadcast[5],
                     result);
    }
    else if (debugEspNow)
        systemPrintf("Added ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                     peerBroadcast[0], peerBroadcast[1],
                     peerBroadcast[2], peerBroadcast[3],
                     peerBroadcast[4], peerBroadcast[5]);
    return result;
}

//*********************************************************************
// ESP-NOW bringup from RTK
bool espNowOn()
{
    uint8_t mode;
    uint8_t primaryChannel;
    uint8_t protocols;
    wifi_second_chan_t secondaryChannel;
    bool started;
    esp_err_t status;

    do
    {
        started = false;

        //   1. Get WiFi mode
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling WiFi.getMode\r\n");
        mode = (uint8_t)WiFi.getMode();
        if (wifiDebug)
            systemPrintf("mode: 0x%08x (%s)\r\n",
                         mode,
                         ((mode == 0) ? "WiFi off"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP"
                         : "STA"))));

        //   2. Set WiFi station mode if necessary
        if ((mode & WIFI_MODE_STA) == 0)
        {
            mode |= WIFI_MODE_STA;
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling WiFi.mode\r\n");
            if (WiFi.mode((wifi_mode_t)mode) == false)
            {
                systemPrintf("ERROR: Failed to set WiFi mode: 0x%08x (%s)\r\n",
                             mode,
                             ((mode == 0) ? "WiFi off"
                             : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA"
                             : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP"
                             : "STA"))));
                break;
            }
        }

        //   3. Get WiFi station protocols
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_wifi_get_protocol\r\n");
        status = esp_wifi_get_protocol(WIFI_IF_STA, &protocols);
        if (wifiDebug)
            systemPrintf("protocols: %d\r\n", protocols);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to read the WiFi station protocols, status: %d\r\n", status);
            break;
        }
    
        //   4. Set WIFI_PROTOCOL_LR protocol
        if ((protocols & WIFI_PROTOCOL_LR) == 0)
        {
            protocols |= WIFI_PROTOCOL_LR;
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling esp_wifi_set_protocol\r\n");
            status = esp_wifi_set_protocol(WIFI_IF_STA, protocols);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to set WIFI_PROTOCOL_LR, status: %d\r\n", status);
                break;
            }
        }

        //   7. Set promiscuous receive callback [esp_wifi_set_promiscuous_rx_cb(promiscuous_rx_cb)]
        //      to get RSSI of action frames
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_wifi_set_promiscuous_rx_cb\r\n");
        status = esp_wifi_set_promiscuous_rx_cb(wifiPromiscuousRxHandler);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to set the WiFi promiscuous RX callback, status: %d\r\n", status);
            break;
        }

        //   6. Call esp_wifi_set_promiscuous(true)
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_wifi_set_promiscuous\r\n");
        status = esp_wifi_set_promiscuous(true);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to set WiFi promiscuous mode, status: %d\r\n", status);
            break;
        }
        
        //   8. Assign a channel if necessary, call espnowSetChannel
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_wifi_get_channel\r\n");
        status = esp_wifi_get_channel(&primaryChannel, &secondaryChannel);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to get the WiFi channels, status: %d\r\n", status);
            break;
        }
        if (wifiDebug)
        {
            systemPrintf("primaryChannel: %d\r\n", primaryChannel);
            systemPrintf("secondaryChannel: %d (%s)\r\n", secondaryChannel,
                         (secondaryChannel == WIFI_SECOND_CHAN_NONE) ? "None"
                         : ((secondaryChannel == WIFI_SECOND_CHAN_ABOVE) ? "Above"
                         : "Below"));
        }
        if (primaryChannel == 0)
        {
            // Select a channel for ESP-NOW
            primaryChannel = 1;
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling esp_wifi_set_channel\r\n");
            status = esp_wifi_set_channel(primaryChannel, secondaryChannel);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to set WiFi primary channel to %d, status: %d\r\n", primaryChannel, status);
                break;
            }
        }

        //   5. Call esp_now_init
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_now_init\r\n");
        status = esp_now_init();
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to initialize ESP-NOW, status: %d\r\n", status);
            break;
        }

        //   9. Set receive callback [esp_now_register_recv_cb(espnowOnDataReceived)]
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_now_register_recv_cb\r\n");
        status = esp_now_register_recv_cb(espNowRxHandler);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to set ESP_NOW RX callback, status: %d\r\n", status);
            break;
        }

        //  10. Add peers from settings
        if (wifiDebug && wifiVerbose)
            systemPrintf("espNowPeerCount: %d\r\n", espNowPeerCount);
        if (espNowPeerCount == 0)
        {
            //  A. If no peers exist
            //      i.   Determine if broadcast peer exists, call esp_now_is_peer_exist
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling esp_now_is_peer_exist\r\n");
            if (esp_now_is_peer_exist(peerBroadcast) == false)
            {
                //  ii.  Add broadcast peer if necessary, call espnowAddPeer
                if (wifiDebug && wifiVerbose)
                    systemPrintf("Calling espNowAddPeer\r\n");
                status = espNowAddPeer(peerBroadcast);
                if (status != ESP_OK)
                {
                    systemPrintf("ERROR: Failed to add ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                                 peerBroadcast[0], peerBroadcast[1],
                                 peerBroadcast[2], peerBroadcast[3],
                                 peerBroadcast[4], peerBroadcast[5],
                                 status);
                    break;
                }
        
                // iii. Tell ESP-NOW that we are looking for peers
                if (wifiDebug && wifiVerbose)
                    systemPrintf("Calling espNowSetState\r\n");
//                espNowSetState(ESPNOW_BROADCASTING);
            }
        }
        else
        {
            int index;

            //  B. If peers exist,
            //      i.  Set ESP-NOW state, call espNowSetState(ESPNOW_PAIRED)
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling espNowSetState\r\n");
//            espNowSetState(ESPNOW_PAIRED);

            //     ii. Loop through peers listed in settings, for each
            for (index = 0; index < espNowPeerCount; index++)
            {
                //      a. Determine if peer exists, call esp_now_is_peer_exist
                if (wifiDebug && wifiVerbose)
                    systemPrintf("Calling esp_now_is_peer_exist\r\n");
                if (esp_now_is_peer_exist(espNowPeer[index]) == false)
                {
                    //  b. Add peer if necessary, call espnowAddPeer
                    if (wifiDebug && wifiVerbose)
                        systemPrintf("Calling espNowAddPeer\r\n");
                    status = espNowAddPeer(&espNowPeer[index][0]);
                    if (status != ESP_OK)
                    {
                        systemPrintf("ERROR: Failed to add ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x, status: %d\r\n",
                                     espNowPeer[0], espNowPeer[1],
                                     espNowPeer[2], espNowPeer[3],
                                     espNowPeer[4], espNowPeer[5], status);
                        break;
                    }
                }

                // Determine if an error occurred
                if (index < espNowPeerCount)
                    break;
            }
        }

        // ESP-NOW has started successfully
        if (wifiDisplay)
            systemPrintf("ESP-NOW online\r\n");
        started = true;
    } while (0);

    // Return the started status
    return started;
}



        //
        // In espnowOnDataReceived
        //  11. Save ESP-NOW RSSI
        //  12. Set lastEspnowRssiUpdate = millis()
        //  13. If in ESPNOW_PAIRING state
        //      A. Validate message CRC
        //      B. If valid CRC
        //          i.  Save peer MAC address
        //          ii. espNowSetState(ESPNOW_MAC_RECEIVED)
        //  14. Else if ESPNOW_MAC_RECEIVED state
        //      A. If ESP-NOW is corrections source, correctionLastSeen(CORR_ESPNOW)
        //          i.  gnss->pushRawData
        //  15. Set espnowIncomingRTCM
        //



//*********************************************************************
// ESP-NOW bringup from RTK
bool espNowOff()
{
    uint8_t mode;
    esp_now_peer_num_t peerCount;
    uint8_t primaryChannel;
    uint8_t protocols;
    wifi_second_chan_t secondaryChannel;
    bool stopped;
    esp_err_t status;

    do
    {
        stopped = false;

        //  10. Set ESP-NOW state, call espnowSetState(ESPNOW_OFF)

        //   3. esp_now_unregister_recv_cb()
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_now_register_recv_cb\r\n");
        status = esp_now_unregister_recv_cb();
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to clear ESP_NOW RX callback, status: %d\r\n", status);
            break;
        }
        if (wifiDebug && wifiVerbose)
            systemPrintf("ESP-NOW: RX callback removed\r\n");

        if (wifiDisplay)
            systemPrintf("ESP-NOW online\r\n");

        //   4. Remove all peers by calling espnowRemovePeer
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_now_is_peer_exist\r\n");
        if (esp_now_is_peer_exist(peerBroadcast))
        {
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling esp_now_del_peer\r\n");
            status = esp_now_del_peer(peerBroadcast);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to delete broadcast peer, status: %d\r\n", status);
                break;
            }
            if (wifiDebug && wifiVerbose)
                systemPrintf("ESP-NOW removed broadcast peer\r\n");
        }

        // Walk the unicast peers
        while (1)
        {
            esp_now_peer_info_t peerInfo;

            // Get the next unicast peer
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling esp_now_fetch_peer\r\n");
            status = esp_now_fetch_peer(true, &peerInfo);
            if (status != ESP_OK)
                break;

            // Remove the unicast peer
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling esp_now_del_peer\r\n");
            status = esp_now_del_peer(peerInfo.peer_addr);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to delete peer %02x:%02x:%02x:%02x:%02x:%02x, status: %d\r\n",
                             peerInfo.peer_addr[0], peerInfo.peer_addr[1],
                             peerInfo.peer_addr[2], peerInfo.peer_addr[3],
                             peerInfo.peer_addr[4], peerInfo.peer_addr[5],
                             status);
                break;
            }
            if (wifiDebug && wifiVerbose)
                systemPrintf("ESP-NOW removed peer %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                             peerInfo.peer_addr[0], peerInfo.peer_addr[1],
                             peerInfo.peer_addr[2], peerInfo.peer_addr[3],
                             peerInfo.peer_addr[4], peerInfo.peer_addr[5]);
        }
        if (status != ESP_ERR_ESPNOW_NOT_FOUND)
        {
            systemPrintf("ERROR: Failed to get peer info for deletion, status: %d\r\n", status);
            break;
        }

        // Get the number of peers
        if (wifiDebug && wifiVerbose)
        {
            systemPrintf("Calling esp_now_get_peer_num\r\n");
            status = esp_now_get_peer_num(&peerCount);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to get the peer count, status: %d\r\n", status);
                break;
            }
            systemPrintf("peerCount: %d\r\n", peerCount);
        }

        //   9. Turn off ESP-NOW. call esp_now_deinit
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_now_deinit\r\n");
        status = esp_now_deinit();
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to deinit ESP-NOW, status: %d\r\n", status);
            break;
        }
        
        //   7. esp_wifi_get_protocol
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_wifi_get_protocol\r\n");
        status = esp_wifi_get_protocol(WIFI_IF_STA, &protocols);
        if (wifiDebug)
            systemPrintf("protocols: %d\r\n", protocols);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to read the WiFi station protocols, status: %d\r\n", status);
            break;
        }

        //   8. Turn off long range protocol if necessary, call esp_wifi_set_protocol
        if ((protocols & WIFI_PROTOCOL_LR) == 0)
        {
            protocols &= ~WIFI_PROTOCOL_LR;
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling esp_wifi_set_protocol\r\n");
            status = esp_wifi_set_protocol(WIFI_IF_STA, protocols);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to turn off WIFI_PROTOCOL_LR, status: %d\r\n", status);
                break;
            }
        }

        //   1. esp_wifi_set_promiscuous(false)
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_wifi_set_promiscuous\r\n");
        status = esp_wifi_set_promiscuous(false);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to clear WiFi promiscuous mode, status: %d\r\n", status);
            break;
        }
        if (wifiDebug && wifiVerbose)
            systemPrintf("WiFi: Disabled promiscuous mode\r\n");

        //   2. esp_wifi_set_promiscuous_rx_cb(nullptr)
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling esp_wifi_set_promiscuous_rx_cb\r\n");
        status = esp_wifi_set_promiscuous_rx_cb(nullptr);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to clear the WiFi promiscuous RX callback, status: %d\r\n", status);
            break;
        }

        //   5. Get WiFi mode
        if (wifiDebug && wifiVerbose)
            systemPrintf("Calling WiFi.getMode\r\n");
        mode = (uint8_t)WiFi.getMode();
        if (wifiDebug)
            systemPrintf("mode: 0x%08x (%s)\r\n",
                         mode,
                         ((mode == 0) ? "WiFi off"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP"
                         : "STA"))));

        //   6. Clear WiFi station mode if necessary
        if (mode & WIFI_MODE_STA)
        {
            mode &= ~WIFI_MODE_STA;
            if (wifiDebug && wifiVerbose)
                systemPrintf("Calling WiFi.mode\r\n");
            if (WiFi.mode((wifi_mode_t)mode) == false)
            {
                systemPrintf("ERROR: Failed to set WiFi mode: 0x%08x (%s)\r\n",
                             mode,
                             ((mode == 0) ? "WiFi off"
                             : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA"
                             : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP"
                             : "STA"))));
                break;
            }
        }

        //  11. Restart WiFi if necessary

        // ESP-NOW has stopped successfully
        if (wifiDisplay)
            systemPrintf("ESP-NOW stopped\r\n");
        stopped = true;
    } while (0);

    // Return the stopped status
    return stopped;
}

//*********************************************************************
// Toggle the ESP-NOW state
void espNowTest()
{
    uint32_t currentMsec;
    static uint32_t lastChangeMsec = -1000 * 1000;

    currentMsec = millis();
    if ((currentMsec - lastChangeMsec) >= (15 * 1000))
    {
        static bool on;

        lastChangeMsec = currentMsec;

        on = !on;
        if (on)
        {
            systemPrintf("--------------------  Start ESP-NOW  ------------------\r\n", rand);
            espNowOn();
        }
        else
        {
            systemPrintf("--------------------  Stop ESP-NOW  -------------------\r\n", rand);
            espNowOff();
        }
    }
}

