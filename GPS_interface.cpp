

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

RTCM3_UBLOX::RTCM3_UBLOX(UnbufferedSerial *uart) : led1(LED1){

    _serial_port = uart; //pointer magic
    rtcm_msg = (uint8_t*) malloc(MAXIMUM_BUFFER_SIZE*MAXIMUM_MESSAGES); //allocate memory in heap (stacksize to small)
    
    clear_buf(rtcm_msg, MAXIMUM_BUFFER_SIZE*MAXIMUM_MESSAGES); //set all to 0 else it will cause issues
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

//to implement in the future
bool RTCM_MSG::checkCRC(){

    return 0;
}


// writes all its contents to an array and clears itself for next msg

bool RTCM_MSG::write2array(uint8_t *buf, uint16_t &len){
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

    clearMsg(); // clear this message (all info is now stored in buf)

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
//init function to be called after creating object
bool RTCM3_UBLOX::init(){
    clearAll(); //reset everythin to make sure all is set to 0
    current_msg = 0;
    reached_max_msg = 0;
    msg_pos = MSG_IDLE;
    isactive = 0;
    c = 0; //clear buffer
    
    t.start();
    _serial_port->attach(callback(this, &RTCM3_UBLOX::rx_interrupt_handler), SerialBase::RxIrq); //start isr
    while(!msg_activity()); //wait a msg cycle else uC could start reading mid reception and create errors
    clearAll();
    return 1;
}


//returns 0 when no activity 1 when data is arriving (theoreticly)
bool RTCM3_UBLOX::msg_activity(){
    //led1 = !led1;

    if(t.elapsed_time() > 1000us){
        //if this triggers message should be recieved
        //please optimize in the futrue
        isactive = 0;
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

// gets message length in bytes
uint16_t RTCM3_UBLOX::getCompleteMsgLength(){
    int i = 0;
    uint16_t sum = 0;
    while(msg[i].isvalid){ //check whether msg accualy has valid data
        sum += msg[i].length + 6; // +6 is from the preamble and crc
        i++;
        if(i >= MAXIMUM_MESSAGES){
            break;
        }
    }
    if(sum != rtcm_msg_length){
        printf("calc are wrong\n sum = %d", sum);
        return 0;
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

    uint16_t length_ = 0;
    uint8_t n = 0;
    //ThisThread::sleep_for(50ms);
    if((msg_pos == MSG_DATA) & !msg_activity()){
        
        //while(msg_activity());
        //printf("artifical bwebrltwebrtlwjhebrtkjwehbrtkwehjrtbwerjkthbwerjthb delay\n");
        if(!decode()){
            //printf("decoder failed\n");
            clearAll();
            return 0;
        }

        n = msg_ready();
        //printf("n = %d\n",n);

        length_ = getCompleteMsgLength();
        
        //printf("0x%x l=%d 0x%x\n", msg[0].preamble, msg[0].length, msg[0].crc);
        for(int i = 0; i < length_; i++){
            buf[i] = rtcm_msg[i];
        }
        len = length_;

        
        clearAll();
        //printf("a = %d , b = %d\n", a,b);
        
    } else {
        return 0;
    }
    return n;
    
}


uint8_t RTCM3_UBLOX::writeCompleteMsg(uint8_t *buf, uint16_t len){
    for(int i = 0; i < len; i++){
        c = buf[i];
        _serial_port->write(&c, 1);

    }

    return 1;
}



bool RTCM3_UBLOX::clearAll(){
    for(int i = 0; i < MAXIMUM_MESSAGES; i++){
        msg[i].clearMsg();
    }
    clear_buf(rtcm_msg, MAXIMUM_BUFFER_SIZE*MAXIMUM_MESSAGES);
    current_msg = 0;
    reached_max_msg = 0;
    rtcm_msg_pointer = 0;
    rtcm_msg_length = 0;
    msg_pos = MSG_IDLE;
    return 1;
}

bool RTCM3_UBLOX::decode(){
    bool loop = true;

    int p_offset = 0; //pointer of the "read array"
    int n = 0; //index to corrent message number
    while(loop){
        msg[n].preamble = rtcm_msg[p_offset+0];
        //printf("n = %d\n",n);
        if(msg[n].preamble != 0xd3){
            printf("preamble wrong\n");
            return 0;
        }
        msg[n].length = ((uint16_t)rtcm_msg[p_offset + 1] << 8) + rtcm_msg[p_offset + 2];
        msg[n].data = (uint8_t*)malloc(msg[n].length);
        for(int i = 0; i < msg[n].length; i++){
            msg[n].data[i] = rtcm_msg[p_offset + 2 + i];
        }
        msg[n].crc = ((uint32_t)rtcm_msg[p_offset + 2 + msg[n].length + 1] << 16)
                    +((uint32_t)rtcm_msg[p_offset + 2 + msg[n].length + 2] <<  8)
                    +((uint32_t)rtcm_msg[p_offset + 2 + msg[n].length + 3]);
        
        if(rtcm_msg[p_offset + 3 + msg[n].length + 3] == 0xd3){
            p_offset = p_offset + 3 + msg[n].length + 3;
            msg[n].isvalid = true;
            //msg[n].checkCRC() //for future implementation
            n++;
        } else {
            loop = 0;
            p_offset = p_offset + 3 + msg[n].length + 3;
            rtcm_msg_length = p_offset;
            msg[n].isvalid = true;
            //printf("total bytes: %d/%d\n", p_offset, rtcm_msg_pointer);

        }

    }
    return 1;
}


// gets called when a byte is ready
void RTCM3_UBLOX::rx_interrupt_handler()
{
    
    if(msg_pos == MSG_ERR){ //stops the function to overfill the buffer if its no called regulary
        return;
    }
    
    
    _serial_port->read(&c,1); //read this byte
    t.reset();                  //reset timer to indicate a read
    isactive = 1;               //set to 1 so we know this function has been called
    rtcm_msg[rtcm_msg_pointer] = c; //sace read message
    rtcm_msg_pointer++;             //advance pointer
    msg_pos = MSG_DATA;             //indicate we are receiving data


    //if pointer gets out of bounds it creates errors so here we set a flag to indicate the buffer is full or faulty
    if(rtcm_msg_pointer > MAXIMUM_BUFFER_SIZE*MAXIMUM_MESSAGES){
        msg_pos = MSG_ERR;
    }
    
    return;

}



//set all values of an array to 0
void RTCM3_UBLOX::clear_buf(uint8_t *buf, int length){
    for(int i = 0; i<length; i++){
        buf[i] = 0;
    }

}