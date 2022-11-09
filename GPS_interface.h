#include <cstdint>
#include <string>
#include "mbed.h"
#include "RTCM3_protocol.h"
#include "UBX_protocol.h"


#define MAXIMUM_RTCM_MESSAGES           10              //should be the same size as expected messages
#define MAXIMUM_UBX_MESSAGES            5               //
#define MAXIMUM_NMEA_MESSAGES           5               //



typedef enum{
    MSG_IDLE,
	MSG_PREAMBLE,
	MSG_LENGTH,
    MSG_DATA,
    MSG_CRC,
    MSG_COMPLETE,
    MSG_ERR
}msg_pos_t;

typedef enum{
    TBD,
    RTCM,
    UBX,
    NMEA
}msg_type_t;


class RTCM3_UBLOX{
    public:
    RTCM3_UBLOX(UnbufferedSerial *uart);
    bool init();

    //some variables
    uint8_t *rtcm_msg;
    uint16_t rtcm_msg_pointer;
    uint16_t rtcm_msg_length;
    bool isactive;

    //classes
    msg_pos_t msg_pos;
    UnbufferedSerial *_serial_port;
    RTCM_MSG msg[MAXIMUM_RTCM_MESSAGES];
    UBX_MSG ubx[MAXIMUM_UBX_MESSAGES];
    Timer t;
    DigitalOut led1;
    bool reached_max_msg;

    //basic functions
    bool msg_activity();
    bool data_ready();
    uint8_t msg_ready(msg_type_t);
    bool printMsgTypes();

    uint8_t writeCompleteMsg(uint8_t *buf, uint16_t len);

    //data handling
    //decodes the recieved msg from internal buffer
    bool decode();
    //encodes all RTCM msgs to an array
    bool encode_RTCM(uint8_t *buf, uint16_t &len);
    //encodes all UBX msgs to an array
    bool encode_UBX(uint8_t *buf, uint16_t &len);

    //clears everything (soft hard reset)
    bool clearAll();

    
    private:
    //UnbufferedSerial *_serial_port;
    void rx_interrupt_handler();
    char c; // most important char ever lol
    



};
void clear_buf(uint8_t *buf, int length);