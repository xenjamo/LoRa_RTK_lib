

#include "GPS_interface.h"
#include <cstdint>
#include <cstdio>

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
        length = 0;
        current_msg_pos = 0;
        crc = 0;
        crc_valid = 0;
        isvalid = 0;
        incoming = 0;
        data = (uint8_t*)malloc(MAXIMUM_BUFFER_SIZE);
}

bool RTCM_MSG::checkCRC(){

    return 0;
}


// writes all its contents to an array and clears itself for next msg

bool RTCM_MSG::write2array(uint8_t *buf, uint16_t &len){
    len = length + 6;
    buf[0] = preamble;
    buf[1] = (uint8_t)(length >> 8);
    buf[2] = (uint8_t) length      ;

    for(int i = 0; i < length; i++){
        buf[3+i] = data[i];
    }
    buf[3+length]   = (uint8_t)(crc >> 16);
    buf[3+length+1] = (uint8_t)(crc >>  8);
    buf[3+length+2] = (uint8_t)(crc >>  0);

    clearMsg();

    return 1;
}
bool RTCM_MSG::clearMsg(){

    preamble = 0;
    length = 0;
    current_msg_pos = 0;
    crc = 0;
    crc_valid = 0;
    isvalid = 0;
    incoming = 0;

    return 1;
}

//////// end of class block ///////////

bool RTCM3_UBLOX::init(){
    clearAll(); //reset everythin to make sure all is set to 0
    current_msg = 0;
    reached_max_msg = 0;
    msg_pos = MSG_PREAMBLE;
    c = 0; //clear buffer
    //_serial_port->format(8,SerialBase::None,1);
    t.start();
    _serial_port->attach(callback(this, &RTCM3_UBLOX::rx_interrupt_handler), SerialBase::RxIrq); //start isr
    while(!msg_activity());
    clearAll();
    return 1;
}

bool RTCM3_UBLOX::msg_activity(){

    if(t.elapsed_time() > 10us){
        return 0;
    }

    return 1;
}



//indicates if RTCM msg hase been recieved and outputs the number uf messages it has stored

uint8_t RTCM3_UBLOX::msg_ready(){
    int i = 0;
    while(msg[i].isvalid){
        i++;
        if(i-1 >= MAXIMUM_MESSAGES){
            return 0;
        }
    }

    if(msg[i].incoming) return 0;

    return i;

}

uint16_t RTCM3_UBLOX::getCompleteMsgLength(){
    int i = 0;
    uint16_t sum = 0;
    while(msg[i].isvalid){
        sum += msg[i].length + 6; // +6 is from the preamble and crc
        i++;
        if(i >= MAXIMUM_MESSAGES){
            break;
        }
    }
    
    return sum;
}


uint8_t RTCM3_UBLOX::readSingleMsg(uint8_t n, uint8_t *buf, uint16_t &len){
    if(!msg[n].isvalid | (n > MAXIMUM_MESSAGES)){
        return 0;
    }

    msg[n].write2array(buf, len);
    
    if(n+1 > MAXIMUM_MESSAGES){
        //check if new msg
        if(msg[n+1].isvalid | msg[n+1].incoming){
            printf("a newer message just came in\n");
            return 2;
        }
    }
    
    return 1;

}

uint8_t RTCM3_UBLOX::readCompleteMsg(uint8_t *buf, uint16_t &len){
    uint8_t n = msg_ready();
    uint16_t length_ = 0;
    len = 0;

    for(int i = 0; i < n; i++){
        msg[i].write2array(buf+len, length_);
        len += length_;
    }
    clearAll();
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
    
    static uint8_t i = 0;
    i = current_msg;
    
    _serial_port->read(&c,1);
    
    t.reset();

    if(reached_max_msg == true){
        msg_pos = MSG_ERR;
    }
    

    switch (msg_pos){
        case(MSG_PREAMBLE):
            if(c == 0xd3){
                //preamble detected
                msg[i].incoming = true;
                msg_pos = MSG_LENGTH;
                msg[i].preamble = c;
                msg[i].current_msg_pos = 1;
            }
        break;
        case(MSG_LENGTH):
            //read msg length
            msg[i].length = msg[i].length | (((uint16_t)c) << 8*msg[i].current_msg_pos);

            if(msg[i].current_msg_pos == 0){
                //LSB
                if(msg[i].length > 1050){
                    msg_pos = MSG_ERR;
                }else{
                    msg_pos = MSG_DATA;
                }


            } else {
                msg[i].current_msg_pos--;
            }

        break;
        case(MSG_DATA):
            //read data
            msg[i].data[msg[i].current_msg_pos] = c;
            msg[i].current_msg_pos++;
            
            if(msg[i].current_msg_pos >= msg[i].length){
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
                msg[i].incoming = false;
                msg[i].isvalid = true;
                
                if(current_msg+1 >= MAXIMUM_MESSAGES){
                    reached_max_msg = true;
                    msg_pos = MSG_PREAMBLE;

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