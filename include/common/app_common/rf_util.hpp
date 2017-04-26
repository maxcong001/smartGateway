struct rf_payload
{
    char protocol;
    char len;
    char data;
};
enum rf_protocol
{
    rf_one_wire_onoff,
    rf_onoff,
    rf_one_wire_analog,
    rf_one_wire,
    rf_i2c,
    rf_spi,
    rf_serial
};
struct one_wire_onoff_s
{
    char pin;
    char data;
    char reserve;
};
struct socket_message
{
    char nodeID;
    char type;
    char len;
    char message;
    const char *toString(void) const;
};
void form_rf_payload(char *buf, char protocol, char *message);
void form_socket_message_from_rf_payload(char *buf, char *rf_payload, char nodeID, char type);
void form_socket_message(char *buf, char nodeID, char type, char protocol, char *message);
