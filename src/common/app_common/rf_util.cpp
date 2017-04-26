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
void form_rf_payload(char *buf, char protocol, char *message)
{
    rf_payload tmp_payload;
    tmp_payload.protocol = protocol;
    tmp_payload.len = sizeof(rf_payload) + get_protocol_len(protocol) - 1;

    memcpy(buf, &tmp_payload, sizeof(tmp_payload));
    memcpy(&(((reinterpret_cast<rf_payload *>(buf))->data)), message, get_protocol_len(protocol));
}
void form_socket_message_from_rf_payload(char *buf, char *payload, char nodeID, char type)
{
    socket_message tmp_socket_message;
    tmp_socket_message.nodeID = nodeID;
    tmp_socket_message.type = type;
    memcpy(buf, &tmp_socket_message, sizeof(socket_message));
    memcpy(&(((reinterpret_cast<socket_message *>(buf))->message)), payload, ((rf_payload *)payload)->len);
}
void form_socket_message(char *buf, char nodeID, char type, char protocol, char *message)
{
    char tmp_buf[32];
    form_rf_payload(tmp_buf, protocol, message);
    form_socket_message_from_rf_payload(buf, tmp_buf, nodeID, type);
}
