

#include <cstdint>
#include <string>


#define MAXIMUM_RTCM_MESSAGE_LENGTH     1+2+1023+3      //preamble + length + maxdata + crc

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