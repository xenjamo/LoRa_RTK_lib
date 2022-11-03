#include <cstdint>
#include "mbed.h"




#define MAXIMUM_BUFFER_SIZE     1+2+1023+3 //preamble + length + maxdata + crc
#define MAXIMUM_MESSAGES        10           //MUST be the same size as expected messages



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
    uint8_t preamble; // in here for completeness sake
    uint16_t length; // length of data
    uint16_t current_msg_pos;
    uint16_t type;
    uint8_t* data;  //stores data
    uint32_t crc; //accually its 24bits
    bool crc_valid; //not implemented
    bool incoming;
    bool isvalid;
    bool checkCRC(); //not implemented
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
    uint8_t *rtcm_msg;
    uint16_t rtcm_msg_pointer;
    uint16_t rtcm_msg_length;
    bool isactive;
    msg_pos_t msg_pos;
    UnbufferedSerial *_serial_port;
    RTCM_MSG msg[MAXIMUM_MESSAGES];
    Timer t;
    DigitalOut led1;
    bool reached_max_msg;
    bool msg_activity();
    uint8_t msg_ready();
    uint16_t getCompleteMsgLength();
    uint8_t readSingleMsg(uint8_t, uint8_t *buf, uint16_t &len);
    uint8_t readCompleteMsg(uint8_t *buf, uint16_t &len);
    uint8_t writeCompleteMsg(uint8_t *buf, uint16_t len);
    bool decode();
    bool clearAll();

    
    private:
    //UnbufferedSerial *_serial_port;
    void rx_interrupt_handler();
    char c; // most important char ever lol
    void clear_buf(uint8_t *buf, int length);



};