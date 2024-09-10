uint8_t networkPriority = NETWORK_OFFLINE;  // Priority of the default network interface

// The following entries have one bit per interface
NetMask_t networkOnline;  // Track the online networks

NetMask_t networkSeqStarting;       // Track the starting sequences
NetMask_t networkSeqStopping;       // Track the stopping sequences
NetMask_t networkSeqNext;           // Determine the next sequence to invoke
NetMask_t networkSeqRequest;        // Request another sequence (bit value 0: stop, 1: start)
NetMask_t networkStarted;           // Track the running networks

// Active network sequence, may be nullptr
NETWORK_POLL_SEQUENCE * networkSequence[NETWORK_OFFLINE];

//----------------------------------------
// Delay for a while
void networkDelay(uint8_t priority, uintptr_t parameter, bool debug)
{
    // Get the timer address
    uint32_t * timer = (uint32_t *)parameter;
    
    // Delay until the timer expires
    if ((int32_t)(millis() - *timer) >= 0)
    {
        // Timer has expired
        networkSequenceNextEntry(priority, debug);
    } 
}

//----------------------------------------
// Get the default network name
const char * networkGetName(uint8_t priority)
{
    if (priority < NETWORK_OFFLINE)
        return networkPriorityTable[priority].name;
    return "None";
}

//----------------------------------------
// Get the current interface name
const char * networkGetCurrentInterface()
{
    return networkGetName(networkPriority);
}

//----------------------------------------
// Determine if any network interface is online
bool networkIsAnyInterfaceOnline()
{
    return networkOnline ? true : false;
}
//----------------------------------------
// Determine if the network is available
bool networkIsInterfaceOnline(uint8_t priority)
{
    return (networkOnline & (1 << priority)) ? true : false;
}

//----------------------------------------
// Determine if the network is available
bool networkIsInternetAvailable(uint8_t * clientPriority)
{
    // If the client is using the highest priority network and that network
    // is still available then continue as normal
    if (networkOnline && (*clientPriority == networkPriority))
        return true;

    // The network has changed, notify the client of the change
    *clientPriority = networkPriority;
    return false;
}

//----------------------------------------
// Determine if the network interface is started
bool networkIsStarted(uint8_t priority)
{
    return (networkStarted & (1 << priority)) ? true : false;
}

//----------------------------------------
// Determine if the network interface is stopped
bool networkIsStopped(uint8_t priority)
{
    return (networkStarted & (1 << priority)) ? false : true;
}

//----------------------------------------
// Mark network offline
void networkMarkOffline(int priority)
{
    NetMask_t bitMask;
    uint8_t previousPriority;

    // Validate the priority
    if (NETWORK_DEBUG_STATE)
        Serial.printf("--------------- %s Offline ---------------\r\n", networkGetName(priority));
    networkPriorityValidation(priority);

    // Mark this network as offline
    networkOnline &= ~(1 << priority);

    // Did the highest priority network just fail?
    if (priority == networkPriority)
    {
        // The highest priority network just failed
        // Leave this network on in hopes that it will regain a connection
        previousPriority = networkPriority;

        // Search in decending priority order for the next online network
        for (priority += 1; priority < NETWORK_OFFLINE; priority += 1)
        {
            // Is the network online?
            bitMask = 1 << priority;
            if (networkOnline & bitMask)
                // Successfully found an online network
                break;

            // No, does this network need starting
            networkStart(priority, NETWORK_DEBUG_SEQUENCE);
        }

        // Set the new network priority
        networkPriority = priority;
        if (priority < NETWORK_OFFLINE)
            Network.setDefaultInterface(*networkPriorityTable[priority].netif);

        // Display the transition
        if (NETWORK_DEBUG_STATE)
            Serial.printf("Default Network Interface: %s --> %s\r\n",
                          networkGetName(previousPriority),
                          networkGetName(priority));
    }
}

//----------------------------------------
// Mark network online
void networkMarkOnline(uint8_t priority)
{
    NetMask_t bitMask;
    uint8_t previousPriority;

    // Validate the priority
    if (NETWORK_DEBUG_STATE)
        Serial.printf("--------------- %s Online ---------------\r\n", networkGetName(priority));
    networkPriorityValidation(priority);

    // Mark this network as online
    networkOnline |= 1 << priority;

    // Raise the network priority if necessary
    previousPriority = networkPriority;
    if (priority < networkPriority)
        networkPriority = priority;

    // The network layer changes the default network interface when a
    // network comes online which can place things out of priority order.
    // Always set the highest priority network as the default
    Network.setDefaultInterface(*networkPriorityTable[networkPriority].netif);

    // Stop lower priority networks when the priority is raised
    if (previousPriority > priority)
    {
        // Display the transition
        Serial.printf("Default Network Interface: %s --> %s\r\n",
                      networkGetName(previousPriority),
                      networkGetName(priority));

        // Set a valid networkPriorityTable entry for previousPriority
        if (previousPriority >= NETWORK_OFFLINE)
            previousPriority = NETWORK_OFFLINE - 1;

        // Stop any lower priority network interfaces
        for (; previousPriority > priority; previousPriority--)
        {
            // Determine if the previous network should be stopped
            bitMask = 1 << previousPriority;
            if (networkPriorityTable[previousPriority].stop
                && (networkStarted & bitMask))
            {
                // Stop the previous network
                Serial.printf("Stopping %s\r\n", networkGetName(previousPriority));
                networkSequenceStop(previousPriority, NETWORK_DEBUG_SEQUENCE);
            }
        }
    }
}

//----------------------------------------
// Process network events
void networkOnEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event)
    {
    default:
        Serial.printf("ERROR: Unknown Arduino event: %d\r\n", event);
        break;

    // Ethernet
    case ARDUINO_EVENT_ETH_START:
    case ARDUINO_EVENT_ETH_CONNECTED:
    case ARDUINO_EVENT_ETH_GOT_IP:
    case ARDUINO_EVENT_ETH_LOST_IP:
    case ARDUINO_EVENT_ETH_DISCONNECTED:
    case ARDUINO_EVENT_ETH_STOP:
        ethernetEvent(event, info);
        break;

    // PPP
    case ARDUINO_EVENT_PPP_START:
    case ARDUINO_EVENT_PPP_CONNECTED:
    case ARDUINO_EVENT_PPP_GOT_IP:
    case ARDUINO_EVENT_PPP_GOT_IP6:
    case ARDUINO_EVENT_PPP_LOST_IP:
    case ARDUINO_EVENT_PPP_DISCONNECTED:
    case ARDUINO_EVENT_PPP_STOP:
        cellularEvent(event);
        break;

    // WiFi
    case ARDUINO_EVENT_WIFI_OFF:
    case ARDUINO_EVENT_WIFI_READY:
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
    case ARDUINO_EVENT_WIFI_STA_START:
    case ARDUINO_EVENT_WIFI_STA_STOP:
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        wifiEvent(event, info);
        break;
    }
}
//----------------------------------------
// Select the next entry in the  sequence
void networkPoll()
{
    uint8_t priority;
    NETWORK_POLL_ROUTINE pollRoutine;
    NETWORK_POLL_SEQUENCE * sequence;

    // Walk the list of priorities in descending order
    for (priority = 0; priority < NETWORK_OFFLINE; priority++)
    {
        // Execute any active polling routine
        sequence = networkSequence[priority];
        if (sequence)
        {
            pollRoutine = sequence->routine;
            if (pollRoutine)
                // Execute the poll routine
                pollRoutine(priority, sequence->parameter, NETWORK_DEBUG_SEQUENCE);
        }
    }
}

//----------------------------------------
// Get the network priority value
// Each network interface should use this routine only once, typically
// during initialization.  After that, use the priority value previously
// assigned.  This allows for a one-time configuration without a corresponding
// enum to match the values in the networkPriorityTable.
uint8_t networkPriorityGet(NetworkInterface *netif)
{
    // Search from highest priority to lowest priority
    for (uint8_t priority = 0; priority < networkPriorityTableEntries; priority++)
        // Return the priority when it is found
        if (networkPriorityTable[priority].netif == netif)
            return priority;

    // No priority found
    Serial.println("HALTED: Network interface missing from networkPriorityTable!");
    while (1)
        ;

    // Priority that indicates no network is online
    return NETWORK_OFFLINE;
}

//----------------------------------------
// Validate the network priority
void networkPriorityValidation(uint8_t priority)
{
    // Validate the priority
    if (priority >= networkPriorityTableEntries)
    {
        Serial.printf("HALTED: Invalid priority value %d, valid range (0 - %d)!\r\n",
                      priority, networkPriorityTableEntries - 1);
        while (1)
            ;
    }
}

//----------------------------------------
// Print the network interface status
void networkPrintStatus(uint8_t priority)
{
    NetMask_t bitMask;
    char highestPriority;
    int index;
    const char * name;
    const char * status;

    // Validate the priority
    networkPriorityValidation(priority);

    // Get the network name
    name = networkGetName(priority);

    // Determine the network status
    bitMask = (1 << priority);
    highestPriority = (networkPriority == priority) ? '*' : ' ';
    status = "Starting";
    if (networkOnline & bitMask)
        status = "Online";
    else if (networkPriorityTable[priority].boot)
    {
        if (networkSeqStopping & bitMask)
            status = "Stopping";
        else if (networkStarted & bitMask)
            status = "Started";
        else
            status = "Stopped";
    }

    // Print the network interface status
    Serial.printf("%c%d: %-10s %-8s\r\n",
                  highestPriority, priority, name, status);
}

//----------------------------------------
// Start the boot sequence
void networkSequenceBoot(uint8_t priority)
{
    NetMask_t bitMask;
    bool debug;
    const char * description;
    NETWORK_POLL_SEQUENCE * sequence;

    // Validate the priority
    networkPriorityValidation(priority);

    // Set the priority bit mask
    bitMask = 1 << priority;
    debug = NETWORK_DEBUG_SEQUENCE;

    // Display the transition
    if (debug)
        Serial.printf("--------------- %s Boot ---------------\r\n", networkGetName(priority));
    if (NETWORK_DEBUG_STATE)
        Serial.printf("%s: Reset --> Booting\r\n", networkGetName(priority));

    // Display the description
    sequence = networkPriorityTable[priority].boot;
    if (sequence)
    {
        description = sequence->description;
        if (debug && description)
            Serial.printf("%s: %s\r\n", networkGetName(priority), description);

        // Start the boot sequence
        networkSequence[priority] = sequence;
        networkSeqStarting &= ~bitMask;
        networkSeqStopping &= ~bitMask;
    }
}

//----------------------------------------
// Select the next entry in the  sequence
void networkSequenceNextEntry(uint8_t priority, bool debug)
{
    NetMask_t bitMask;
    const char * description;
    NETWORK_POLL_SEQUENCE * next;
    bool start;

    // Validate the priority
    networkPriorityValidation(priority);

    // Get the previous sequence entry
    next = networkSequence[priority];

    // Set the next sequence entry
    next += 1;
    if (next->routine)
    {
        // Display the description
        description = next->description;
        if (debug && description)
            Serial.printf("%s: %s\r\n", networkGetName(priority), description);

        // Start the next entry in the sequence
        networkSequence[priority] = next;
    }

    // Termination entry found, stop the sequence or start next sequence
    else
    {
        // Stop the polling for this sequence
        networkSequence[priority] = nullptr;
        bitMask = 1 << priority;

        // Display the transition
        if (NETWORK_DEBUG_STATE && (networkSeqStarting & bitMask))
            Serial.printf("%s: Starting --> Started\r\n", networkGetName(priority));
        else if (NETWORK_DEBUG_STATE && (networkSeqStarting & bitMask))
            Serial.printf("%s: Stopping --> Stopped\r\n", networkGetName(priority));
        else
            Serial.printf("%s: Booting --> Booted\r\n", networkGetName(priority));

        // Clear the status bits
        networkSeqStarting &= ~bitMask;
        networkSeqStopping &= ~bitMask;

        // Check for another sequence request
        if (networkSeqRequest & bitMask)
        {
            // Another request is pending, get the next request
            start = networkSeqNext & bitMask;

            // Clear the bits
            networkSeqRequest &= ~bitMask;
            networkSeqNext &= ~bitMask;
            networkSeqRequest &= ~bitMask;

            // Start the next sequence
            if (networkSeqNext & bitMask)
                networkSequenceStart(priority, debug);
            else
                networkSequenceStop(priority, debug);
        }
    }
}

//----------------------------------------
// Attempt to start the start sequence
void networkSequenceStart(uint8_t priority, bool debug)
{
    NetMask_t bitMask;
    const char * description;
    NETWORK_POLL_SEQUENCE * sequence;

    // Validate the priority
    networkPriorityValidation(priority);

    // Set the priority bit mask
    bitMask = 1 << priority;

    // Determine if the sequence any sequence is already running
    sequence = networkSequence[priority];
    if (sequence)
    {
        // A sequence is already running, set the next request
        // Check for already starting
        if (networkSeqStarting & bitMask)
        {
            if (debug)
                Serial.printf("%s sequencer running, dropping request since already starting\r\n",
                    networkGetName(priority));

            // Ignore this request, compressed
            // Compress multiple requests
            //   Start --> Start = Start
            //   Start --> Stop --> ... --> Stop --> Start = Start
        }

        // Either boot request or stop request is running
        else
        {
            if (debug)
                Serial.printf("%s sequencer running, delaying start sequence\r\n",
                    networkGetName(priority));

            // Compress multiple requests
            //   Boot --> Start
            //   Stop --> Start = Start
            //   Stop --> Start -> ... --> Stop --> Start  = Start
            // Note the next request to execute
            networkSeqNext |= bitMask;
            networkSeqRequest |= bitMask;
        }
    }
    else
    {
        // No sequence is running
        if (debug)
            Serial.printf("%s sequencer idle\r\n", networkGetName(priority));

        // Display the transition
        if (debug)
            Serial.printf("--------------- %s Start ---------------\r\n", networkGetName(priority));
        if (NETWORK_DEBUG_STATE)
            Serial.printf("%s: Stopped --> Starting\r\n", networkGetName(priority));

        // Display the description
        sequence = networkPriorityTable[priority].start;
        if (sequence)
        {
            description = sequence->description;
            if (debug && description)
                Serial.printf("%s: %s\r\n", networkGetName(priority), description);

            // Start the sequence
            networkSequence[priority] = sequence;
            networkSeqStarting |= bitMask;
            networkStarted |= bitMask;
        }
    }
}

//----------------------------------------
// Start the boot sequence
void networkSequenceStop(uint8_t priority, bool debug)
{
    NetMask_t bitMask;
    const char * description;
    NETWORK_POLL_SEQUENCE * sequence;

    // Validate the priority
    networkPriorityValidation(priority);

    // Set the priority bit mask
    bitMask = 1 << priority;

    // Determine if the sequence any sequence is already running
    sequence = networkSequence[priority];
    if (sequence)
    {
        // A sequence is already running, set the next request
        // Check for already stopping
        if (networkSeqStopping & bitMask)
        {
            if (debug)
                Serial.printf("%s sequencer running, dropping request since already stopping\r\n",
                    networkGetName(priority));

            // Ignore this request, compressed
            // Compress multiple requests
            //   Stop --> Stop = Stop
            //   Stop --> Start --> ... --> Start --> Stop = Stop
        }

        // Either boot request or start request is running
        else
        {
            if (debug)
                Serial.printf("%s sequencer running, delaying stop sequence\r\n",
                    networkGetName(priority));

            // Compress multiple requests
            //   Boot ---> Stop
            //   Start --> Stop = Stop
            //   Start --> Stop -> ... --> Start --> Stop  = Stop
            // Note the next request to execute
            networkSeqNext &= ~bitMask;
            networkSeqRequest |= bitMask;
        }
    }
    else
    {
        // No sequence is running
        if (debug)
            Serial.printf("%s sequencer idle\r\n", networkGetName(priority));

        // Display the transition
        if (debug)
            Serial.printf("--------------- %s Stop ---------------\r\n", networkGetName(priority));
        if (NETWORK_DEBUG_STATE)
            Serial.printf("%s: Started --> Stopped\r\n", networkGetName(priority));

        // Display the description
        sequence = networkPriorityTable[priority].stop;
        if (sequence)
        {
            description = sequence->description;
            if (debug && description)
                Serial.printf("%s: %s\r\n", networkGetName(priority), description);

            // Start the sequence
            networkSeqStopping |= bitMask;
            networkStarted &= ~bitMask;
            networkSequence[priority] = sequence;
        }
    }
}

//----------------------------------------
// Start a network interface
void networkStart(uint8_t priority, bool debug)
{
    NetMask_t bitMask;

    // Validate the priority
    networkPriorityValidation(priority);

    bitMask = (1 << priority);
    if (networkPriorityTable[priority].start
        && (!(networkStarted & bitMask)))
            Serial.printf("Starting %s\r\n", networkGetName(priority));
        networkSequenceStart(priority, debug);
}

//----------------------------------------
// Start the network if only lower priority networks started at boot
void networkStartDelayed(uint8_t priority, uintptr_t parameter, bool debug)
{
    NetMask_t bitMask;
    const char * currentInterface;
    NetMask_t higherPriorityInterfaces;
    int index;
    const char * name;
    const char * status;

    // Handle the boot case where only lower priority network interfaces
    // start.  In this case, a start is never issued to the cellular layer
    if (millis() >= parameter)
    {
        // Set the next state
        networkSequenceNextEntry(priority, debug);

        // Display the network interfaces
        for (index = 0; index < NETWORK_OFFLINE; index++)
            networkPrintStatus(index);
        
        // Only lower priority networks running, start this network interface
        bitMask = (1 << NETWORK_OFFLINE) -1;
        bitMask &= ~((1 << priority) - 1);
        higherPriorityInterfaces = networkOnline & ~bitMask;
        name = networkGetName(priority);
        currentInterface = networkGetCurrentInterface();
        if (!higherPriorityInterfaces)
        {
            if (debug)
                Serial.printf("%s online, Starting %s\r\n",
                              currentInterface, name);

            // Only lower priority interfaces or none running
            // Start this network interface
            networkStart(priority, NETWORK_DEBUG_SEQUENCE);
        }
        else if (debug)
            Serial.printf("%s online, leaving %s off\r\n",
                          currentInterface, name);
    }
    else if (debug)
    {
        // Count down the delay
        int32_t seconds = millis() / 1000;
        static int32_t previousSeconds = -1;
        if (previousSeconds == -1)
            previousSeconds = parameter / 1000;
        if (seconds != previousSeconds)
        {
            previousSeconds = seconds;
            Serial.printf("Delaying Start: %d Sec\r\n", (parameter / 1000) - seconds);
        }
    }
}

