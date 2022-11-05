
#include "RTCM3_protocol.h"

//////// class block ///////////

RTCM_MSG::RTCM_MSG(){
        preamble = 0;
        length = 0;
        current_msg_pos = 0;
        crc = 0;
        crc_valid = 0;
        isvalid = 0;
        incoming = 0;
        data = (uint8_t*)malloc(MAXIMUM_RTCM_MESSAGE_LENGTH);
}

//to implement in the future
bool RTCM_MSG::checkCRC(){

    return 0;
}


// writes all its contents to an array and clears itself for next msg

void RTCM_MSG::encode(uint8_t *buf, uint16_t &len){
    //message inforamtions in the first 4 bytes
    len = length + 6; 
    buf[0] = preamble;
    buf[1] = (uint8_t)(length >> 8);
    buf[2] = (uint8_t) length      ;

    //copy data to the new array // may use memcpy in the future
    for(int i = 0; i < length; i++){
        buf[3+i] = data[i];
    }

    //write CRC to last 3 bytes
    buf[3+length]   = (uint8_t)(crc >> 16);
    buf[3+length+1] = (uint8_t)(crc >>  8);
    buf[3+length+2] = (uint8_t)(crc >>  0);

    clear(); // clear this message (all info is now stored in buf)

}

uint16_t RTCM_MSG::getlength(){
    return length + 6;
}

bool RTCM_MSG::clear(){
    std::fill_n(data, length, 0);
    preamble = 0;
    length = 0;
    current_msg_pos = 0;
    type = 0;
    crc = 0;
    crc_valid = 0;
    isvalid = 0;
    incoming = 0;
    return 1;
}

//////// end of class block ///////////