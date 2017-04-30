#include "rf_util.hpp"
#include <stdio.h>
#include <string.h>

const char *socket_message::toString(void) const
{
    static char buffer[200];
    sprintf(buffer, "id: %d, type: %d, len: %d", nodeID, type, len);
    return buffer;
}
char get_protocol_len(char protocol)
{
    switch (protocol)
    {
    case rf_one_wire_onoff:
        return sizeof(one_wire_onoff_s);
        break;
    default:
        return 0;
        break;
    }
}
//void form_rf_payload(char *buf, char protocol, char *message)
void form_rf_payload(char *buf, rf24_protocol protocol, protocol_detail protocol_detail)
{
    if (!buf)
    {
        return;
    }
    memcpy(&(((reinterpret_cast<rf24_msg *>(buf))->_protocol)), protocol, sizeof(protocol));
    memcpy(&(((reinterpret_cast<rf24_msg *>(buf))->_protocol_detail)), protocol_detail, sizeof(protocol_detail));
}
void form_socket_message_from_rf_payload(char *buf, char *rf_payload, uint16_t nodeID, char type);
//void form_socket_message_from_rf_payload(char *buf, char *payload, char nodeID, char type)
{
    socket_message* tmp_socket_message = reinterpret_cast<socket_message *>(buf);
    tmp_socket_message->nodeID = nodeID;
    tmp_socket_message->type = type;

    memcpy(&(tmp_socket_message->msg), rf_payload, sizeof(rf24_msg));
}
void form_socket_message(char *buf, uint16_t nodeID, char type, rf24_protocol protocol, protocol_detail protocol_detail)
//void form_socket_message(char *buf, char nodeID, char type, char protocol, char *message)
{
    char tmp_buf[32];
    form_rf_payload(tmp_buf, protocol, protocol_detail);
    form_socket_message_from_rf_payload(buf, tmp_buf, nodeID, type);
}
