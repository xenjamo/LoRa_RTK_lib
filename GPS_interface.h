#include <cstdint>
#include <string>
#include "mbed.h"




#define MAXIMUM_RTCM_MESSAGE_LENGTH     1+2+1023+3      //preamble + length + maxdata + crc
#define MAXIMUM_RTCM_MESSAGES           10              //should be the same size as expected messages
#define MAXIMUM_UBX_MESSAGES            5               //
#define MAXIMUM_NMEA_MESSAGES



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
    void encode(uint8_t *buf, uint16_t &len);
    uint16_t getlength();
    bool clear();
    private:
    //nothing private yet
};

class UBX_MSG{

    public:
    UBX_MSG();
    uint16_t header;
    uint8_t _class;
    uint8_t id;
    uint16_t length;
    uint8_t *data;
    bool isvalid;
    uint8_t ch_[2];
    void encode(uint8_t *buf, uint16_t &len);
    uint16_t getlength();
    string tostring();
    bool clear();
    private:

};

class RTCM3_UBLOX{
    public:
    RTCM3_UBLOX(UnbufferedSerial*);
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
    void printMsgTypes();

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