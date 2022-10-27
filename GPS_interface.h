#include <cstdint>
#include "mbed.h"




#define MAXIMUM_BUFFER_SIZE     1+2+1023+3 //preamble + length + maxdata + crc
#define MAXIMUM_MESSAGES        4           //MUST be the same size as expected messages



typedef enum{
    MSG_IDLE,
	MSG_PREAMBLE,
	MSG_LENGTH,
    MSG_DATA,
    MSG_CRC,
    MSG_COMPLETE,
    MSG_ERR
}msg_pos_t;

class RTCM_MSG{

    public:
    RTCM_MSG();
    uint8_t preamble;
    uint16_t msg_length;
    uint16_t current_msg_pos;
    uint8_t data[MAXIMUM_BUFFER_SIZE];
    uint32_t crc; //accually its 24bits
    bool crc_valid;
    bool msg_incoming;
    bool msg_valid;
    bool checkCRC();
    bool write2array(uint8_t *buf, uint16_t &len);
    bool clearMsg();
    private:
    //nothing private yet
};

class RTCM3_UBLOX{
    public:
    RTCM3_UBLOX(UnbufferedSerial*);


    bool init();
    uint8_t current_msg;
    msg_pos_t msg_pos;
    UnbufferedSerial *_serial_port;
    RTCM_MSG msg[MAXIMUM_MESSAGES];
    bool reached_max_msg;
    uint8_t msg_ready();
    uint8_t readMsg(uint8_t, uint8_t *buf, uint16_t &len);
    bool clearAll();

    
    private:
    //UnbufferedSerial *_serial_port;
    void rx_interrupt_handler();
    char c; // most important char ever lol



};