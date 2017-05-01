#include "time.h"
#include <stdint.h>

#define RF24_SERVER_PORT 25341
#define RF24_SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

//time_t time(time_t * timer)
#define MAGIC_NUM 'M'
struct i2c_detail
{

};
struct pin_detail
{
//    pin_detail():pin(0),on_off(0){}
    char pin;
    char on_off;
};
struct spi_detail
{

};
struct one_wire_detail
{

};
// for rf24l01 message protocol type

enum rf24_protocol
{
    rf24_protocol_pin = 0,
    rf24_protocol_i2c,
    rf24_protocol_spi,
    rf24_protocol_one_wire 
};

union protocol_detail
{
    i2c_detail _i2c;
    pin_detail _pin;
    spi_detail _spi;
    one_wire_detail _one_wire;
};

// the message contains the following part
// 1. rf24_protocol
// 2. protocol_detail
// message is 8
struct rf24_msg
{
    rf24_protocol _protocol;
    protocol_detail _protocol_detail;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// message related to DB
enum rf24_db_msg_type
{
    rf24_db_msg_type_pin=0,
    rf24_db_msg_type_temperature,
    rf24_db_msg_type_humidity

};
struct rf24_db_msg_pin_detail
{
    char on_off;
};
struct rf24_db_msg_temperature_detail
{
    int _temperature;
};
struct rf24_db_msg_humidity_detail
{
    int _humidity;
};

union rf24_db_msg_detail
{
    rf24_db_msg_pin_detail _pin_detail;
    rf24_db_msg_temperature_detail _temperature_detail;
    rf24_db_msg_humidity_detail _humidity_detail;
};

struct rf24_db_msg_value
{
    time_t _time;
    rf24_db_msg_type _type;
    rf24_db_msg_detail _detail;  
};
// message size is 16
struct rf24_db_msg
{
    char key;    // the key is node ID
    rf24_db_msg_value _value;  
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

struct socket_message
{
    char magic_num;
    char type;
    uint16_t nodeID;
    rf24_msg msg;
};

void form_rf_payload(char *buf, rf24_protocol _protocol, protocol_detail* _protocol_detail);
void form_socket_message_from_rf_payload(char *buf, char *rf_payload, uint16_t nodeID, char type);
void form_socket_message(char *buf, uint16_t nodeID, char type, rf24_protocol _protocol, protocol_detail* _protocol_detail);
