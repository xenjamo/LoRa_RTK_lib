

#include "GPS_interface.h"

/*
 * Copyright (c) 2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

// Create a DigitalOutput object to toggle an LED whenever data is received.

// Create a UnbufferedSerial object with a default baud rate.
//static UnbufferedSerial serial_port(PA_0, PA_1);

RTCM3_UBLOX::RTCM3_UBLOX(UnbufferedSerial *uart){

    _serial_port = uart;
}


//////// nested class block ///////////
RTCM3_UBLOX::RTCM_MSG::RTCM_MSG(){
        preamble = 0;
        msg_length = 0;
        current_msg_pos = 0;
        crc = 0;
        crc_valid = 0;
        msg_valid = 0;
}

bool RTCM3_UBLOX::RTCM_MSG::checkCRC(){

    return 0;
}
bool RTCM3_UBLOX::RTCM_MSG::write2array(uint8_t *buf, uint8_t *len){

    return 0;
}
bool RTCM3_UBLOX::RTCM_MSG::clearMsg(){
    
    preamble = 0;
    msg_length = 0;
    current_msg_pos = 0;
    crc = 0;
    crc_valid = 0;
    msg_valid = 0;

    return 0;
}

//////// end of nested class block ///////////

bool RTCM3_UBLOX::init_(){
    clearAll();
    current_msg = 0;
    msg_pos = PREAMBLE; // make int ready to recieve stuff
    c = 0; //clear buffer
}


bool RTCM3_UBLOX::clearAll(){
    for(int i = 0; i < MAXIMUM_MESSAGES; i++){
        msg[i].clearMsg();
        
    }
    return 1;
}


void RTCM3_UBLOX::rx_interrupt_handler()
{
    uint8_t i = current_msg;

    _serial_port->read(&c,1);
    if(msg[0].msg_valid){
        i++;
        current_msg++;
    }

    switch (msg_pos){
        case(PREAMBLE):
            if(c == 0xd3){
                //preamble detected
                msg_pos = MSG_LENGTH;
                msg[0].preamble = c;
                msg[0].current_msg_pos = 1;
            }
        break;
        case(MSG_LENGTH):
            //read msg length
            msg[0].msg_length = msg[0].msg_length | (((uint16_t)c) << 8*msg[0].current_msg_pos);

            if(msg[0].current_msg_pos == 0){
                //LSB
                msg_pos = DATA;

            } else {
                msg[0].current_msg_pos--;
            }

        break;
        case(DATA):
            //read data
            msg[0].data[msg[0].current_msg_pos] = c;
            msg[0].current_msg_pos++;
            if(msg[0].current_msg_pos == msg[0].msg_length){
                msg[0].current_msg_pos = 2;
                msg_pos = CRC_;
            }

        break;
        case(CRC_):
            //read crc
            msg[0].crc = msg[0].crc | (((uint32_t)c) << 8*msg[0].current_msg_pos);

            if(msg[0].current_msg_pos == 0){
                //LSB
                msg_pos = PREAMBLE;
                msg[0].msg_valid = true;

            } else {
                msg[0].current_msg_pos--;
            }

        break;
        default:
            // nothing
        break;
    }


}


void print_hex(const char *s, int len)
{
    while(len--){
        printf("%02x", (unsigned int) *s++);
    }
}

void clear_buf(uint8_t *buf, int length){
    for(int i = 0; i<length; i++){
        buf[i] = 0;
    }

}