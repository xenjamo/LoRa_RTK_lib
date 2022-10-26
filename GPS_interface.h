#include <cstdint>
#include "mbed.h"




#define MAXIMUM_BUFFER_SIZE     1+2+1023+3 //preamble + length + maxdata + crc
#define MAXIMUM_MESSAGES        10



typedef enum{
    IDLE,
	PREAMBLE,
	MSG_LENGTH,
    DATA,
    CRC_,
    MSG_COMPLETE,
    MSG_ERR
}msg_pos_t;

class RTCM3_UBLOX{
    public:
    RTCM3_UBLOX(UnbufferedSerial*);

    class RTCM_MSG{
        public:
        RTCM_MSG();
        uint8_t preamble;
        uint16_t msg_length;
        uint16_t current_msg_pos;
        uint8_t data[MAXIMUM_BUFFER_SIZE];
        uint32_t crc; //accually its 24bits
        bool crc_valid;
        bool msg_valid;
        bool checkCRC();
        bool write2array(uint8_t *buf, uint8_t *len);
        bool clearMsg();
        private:
        //nothing private yet
        };

    bool init_();
    char c; // most important char ever lol
    uint8_t current_msg;
    msg_pos_t msg_pos;
    RTCM_MSG msg[MAXIMUM_MESSAGES];
    bool clearAll();

    
    private:
    UnbufferedSerial *_serial_port;
    void rx_interrupt_handler();



};