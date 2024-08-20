//----------------------------------------
void networkOnEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event)
    {
    default:
        break;

    case ARDUINO_EVENT_ETH_START:
    case ARDUINO_EVENT_ETH_CONNECTED:
    case ARDUINO_EVENT_ETH_GOT_IP:
    case ARDUINO_EVENT_ETH_LOST_IP:
    case ARDUINO_EVENT_ETH_DISCONNECTED:
    case ARDUINO_EVENT_ETH_STOP:
        ethernetEvent(event, info);
        break;
    }
}
