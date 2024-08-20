//----------------------------------------
// Process network events
void networkOnEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    IPAddress ipv4;

    switch (event)
    {
    default:
        if (event != ARDUINO_EVENT_PPP_GOT_IP6)
            Serial.printf("ERROR: Unknown Arduino event: %d\r\n", event);
        break;

    case ARDUINO_EVENT_PPP_START:
    case ARDUINO_EVENT_PPP_CONNECTED:
    case ARDUINO_EVENT_PPP_GOT_IP:
    case ARDUINO_EVENT_PPP_LOST_IP:
    case ARDUINO_EVENT_PPP_DISCONNECTED:
    case ARDUINO_EVENT_PPP_STOP:
        pppEvent(event);
        break;
    }
}
