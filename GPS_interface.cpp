

#include "GPS_interface.h"
#include <cstdint>

/*
 * Copyright (c) 2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

// Create a DigitalOutput object to toggle an LED whenever data is received.

// Create a UnbufferedSerial object with a default baud rate.
//static UnbufferedSerial serial_port(PA_0, PA_1, 921600);

RTCM3_UBLOX::RTCM3_UBLOX(UnbufferedSerial *uart){

    _serial_port = uart; //pointer magic
    
}


//////// class block ///////////


RTCM_MSG::RTCM_MSG(){
        preamble = 0;
        msg_length = 0;
        current_msg_pos = 0;
        crc = 0;
        crc_valid = 0;
        msg_valid = 0;
}

bool RTCM_MSG::checkCRC(){

    return 0;
}


// writes all its contents to an array and clears itself for next msg

bool RTCM_MSG::write2array(uint8_t *buf, uint16_t &len){
    len = msg_length + 6;
    buf[0] = preamble;
    buf[1] = (uint8_t)(msg_length >> 8);
    buf[2] = (uint8_t) msg_length      ;

    for(int i = 0; i < msg_length; i++){
        buf[3+i] = data[i];
    }
    buf[3+msg_length]   = (uint8_t)(crc >> 16);
    buf[3+msg_length+1] = (uint8_t)(crc >>  8);
    buf[3+msg_length+2] = (uint8_t) crc       ;

    clearMsg();

    return 0;
}
bool RTCM_MSG::clearMsg(){

    preamble = 0;
    msg_length = 0;
    current_msg_pos = 0;
    crc = 0;
    crc_valid = 0;
    msg_valid = 0;

    return 0;
}

//////// end of class block ///////////

bool RTCM3_UBLOX::init(){
    clearAll(); //reset everythin to make sure all is set to 0
    current_msg = 0;
    msg_pos = MSG_PREAMBLE; // make interrupt ready to recieve stuff
    c = 0; //clear buffer
    //_serial_port->format(8,SerialBase::None,1);
    _serial_port->attach(callback(this, &RTCM3_UBLOX::rx_interrupt_handler), SerialBase::RxIrq); //start isr
    return 0;
}


//indicates if RTCM msg hase been recieved and outputs the number uf messages it has stored

uint8_t RTCM3_UBLOX::msg_ready(){
    if(msg[current_msg].msg_valid){
        return current_msg+1;
    }else{
        return 0;
    }
}

uint8_t RTCM3_UBLOX::readMsg(uint8_t n, uint8_t *buf, uint16_t &len){
    if(!msg[n].msg_valid | (n > MAXIMUM_MESSAGES)){
        return 0;
    }

    msg[n].write2array(buf, len);
    
    if(n+1 > MAXIMUM_MESSAGES){
        //check if new msg
        if(msg[n+1].msg_valid | msg[n+1].msg_incoming){
            printf("a newer message just came in\n");
            return 2;
        }
    }
    
    return 1;

}


bool RTCM3_UBLOX::clearAll(){
    for(int i = 0; i < MAXIMUM_MESSAGES; i++){
        msg[i].clearMsg();
    }
    current_msg = 0;
    reached_max_msg = 0;
    return 1;
}




// private
void RTCM3_UBLOX::rx_interrupt_handler()
{
    
    uint8_t i = current_msg;
    
    _serial_port->read(&c,1);
    

    switch (msg_pos){
        case(MSG_PREAMBLE):
            if(c == 0xd3){
                //preamble detected
                msg[i].msg_incoming = true;
                msg_pos = MSG_LENGTH;
                msg[i].preamble = c;
                msg[i].current_msg_pos = 1;
            }
        break;
        case(MSG_LENGTH):
            //read msg length
            msg[i].msg_length = msg[i].msg_length | (((uint16_t)c) << 8*msg[i].current_msg_pos);

            if(msg[i].current_msg_pos == 0){
                //LSB
                msg_pos = MSG_DATA;

            } else {
                msg[i].current_msg_pos--;
            }

        break;
        case(MSG_DATA):
            //read data
            msg[i].data[msg[i].current_msg_pos] = c;
            msg[i].current_msg_pos++;
            if(msg[i].current_msg_pos == msg[i].msg_length){
                msg[i].current_msg_pos = 2;
                msg_pos = MSG_CRC;
            }

        break;
        case(MSG_CRC):
            //read crc
            msg[i].crc = msg[i].crc | (((uint32_t)c) << 8*msg[i].current_msg_pos);

            if(msg[i].current_msg_pos == 0){
                //LSB
                msg_pos = MSG_PREAMBLE;
                msg[i].msg_incoming = false;
                msg[i].msg_valid = true;
                
                if(current_msg+1 >= MAXIMUM_MESSAGES){
                    reached_max_msg = true;
                }else{
                    current_msg++;
                }

                

            } else {
                msg[i].current_msg_pos--;
            }

        break;
        default:
            // nothing
        break;
    }


}




void clear_buf(uint8_t *buf, int length){
    for(int i = 0; i<length; i++){
        buf[i] = 0;
    }

}