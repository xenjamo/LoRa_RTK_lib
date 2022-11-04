

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
    rtcm_msg = (uint8_t*) malloc(2000); //allocate memory in heap (stacksize to small)
    
    clear_buf(rtcm_msg, MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES); //set all to 0 else it will cause issues
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
//////// other class block //////////

UBX_MSG::UBX_MSG(){
    header = 0;
    _class = 0;
    id = 0;
    length = 0;
    isvalid = 0;
    data = (uint8_t*)calloc(5,1);

}
void UBX_MSG::encode(uint8_t *buf, uint16_t &len){
    len = length + 5;
    buf[0] = header >> 8;
    buf[1] = header;

    buf[2] = _class;
    buf[3] = id;

    buf[4] = length;
    for(int i = 0; i < length; i++){
        buf[5+i] = data[i];
    }
    clear();
}

uint16_t UBX_MSG::getlength(){
    return length + 5; //not entirely correct some UBX msgs can have diffrent structures please keep that in mind
}

bool UBX_MSG::clear(){
    header = 0;
    _class = 0;
    id = 0;
    length = 0;
    isvalid = 0;
    return 1;
}

////// end of BX msg block//////////

//init function to be called after creating object
bool RTCM3_UBLOX::init(){
    clearAll(); //reset everythin to make sure all is set to 0
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
        //please optimize in the future
        isactive = 0;
        return 0;
    }
    return 1;
}
bool RTCM3_UBLOX::data_ready(){
    return (msg_pos == MSG_DATA) & !msg_activity();
}



//indicates if RTCM msg hase been recieved and outputs the number uf messages it has stored

uint8_t RTCM3_UBLOX::msg_ready(msg_type_t t){
    int i = 0;
    switch(t){
        case(TBD):
        printf("you shouldn't use this\n");
        return 0;
        break;
        case(RTCM):
        while(msg[i].isvalid){
            i++;
            if(i > MAXIMUM_RTCM_MESSAGES){
                printf("this should not be possible\n");
                return 0;
            }
        }
        break;
        case(UBX):
        while(ubx[i].isvalid){
            i++;
            if(i > MAXIMUM_RTCM_MESSAGES){
                printf("this should not be possible\n");
                return 0;
            }
        }
        break;
        case(NMEA):
        printf("no.\n");
        break;
        default:
        printf("congrats you broke my code\n");
        break;
    }
    
    return i;

}


uint8_t RTCM3_UBLOX::writeCompleteMsg(uint8_t *buf, uint16_t len){
    for(int i = 0; i < len; i++){
        c = buf[i];
        _serial_port->write(&c, 1);

    }

    return 1;
}



bool RTCM3_UBLOX::clearAll(){
    for(int i = 0; i < MAXIMUM_RTCM_MESSAGES; i++){
        msg[i].clear();
    }
    clear_buf(rtcm_msg, MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES);
    reached_max_msg = 0;
    rtcm_msg_pointer = 0;
    rtcm_msg_length = 0;
    msg_pos = MSG_IDLE;
    return 1;
}

bool RTCM3_UBLOX::decode(){
    bool loop = true;

    int p_offset = 0; //pointer of the "read array"
    uint8_t first_byte = 0;
    int n = 0; //index to current message number
    int k = 0; //index to current ubx msg
    msg_type_t type = TBD;

    while(loop){

        first_byte = rtcm_msg[p_offset+0];

        //printf("n = %d\n",n);
        if(first_byte == 0xd3){
            msg[n].clear();
            free(msg[n].data);
            msg[n].preamble = rtcm_msg[p_offset+0];
            msg[n].length = ((uint16_t)rtcm_msg[p_offset + 1] << 8) + rtcm_msg[p_offset + 2];
            msg[n].data = (uint8_t*)calloc(msg[n].length, 1);
            for(int i = 0; i < msg[n].length; i++){
                msg[n].data[i] = rtcm_msg[p_offset + 2 + i];
            }
            msg[n].type = (((uint16_t)rtcm_msg[p_offset + 3] << 8) + rtcm_msg[p_offset + 4])  >> 4;
            msg[n].crc = ((uint32_t)rtcm_msg[p_offset + 2 + msg[n].length + 1] << 16)
                        +((uint32_t)rtcm_msg[p_offset + 2 + msg[n].length + 2] <<  8)
                        +((uint32_t)rtcm_msg[p_offset + 2 + msg[n].length + 3]);
            //
            p_offset = p_offset + 3 + msg[n].length + 3;
            msg[n].isvalid = true;
            n++;

        }else if(first_byte == 0xb5){
            ubx[k].clear();
            free(ubx[k].data);
            ubx[k].header = ((uint16_t)rtcm_msg[p_offset+0] << 8) + rtcm_msg[p_offset+1];
            ubx[k]._class = rtcm_msg[p_offset+2];
            ubx[k].id = rtcm_msg[p_offset+3];
            ubx[k].length = rtcm_msg[p_offset+4];
            ubx[k].data = (uint8_t*)calloc(ubx[k].length, 1);
            for(int i = 0; i < ubx[k].length; i++){
                ubx[k].data[i] = rtcm_msg[p_offset+5+i];
            }
            p_offset = p_offset + 5 + ubx[k].length;
            ubx[k].isvalid = true;


        }else if(first_byte == '$'){
            printf("a NMEA msg\n");
            return 0;

        }else if(first_byte == 0){
            rtcm_msg_length = p_offset;
            loop = 0;
        }else{
            rtcm_msg_length = p_offset;
            loop = 0;
            printf("preamble wrong\n");
            return 0;
        }
        
        
        

    }
    return 1;
}

bool RTCM3_UBLOX::encode_RTCM(uint8_t *buf, uint16_t &len){
    int i = msg_ready(RTCM);
    if(i == 0){
        printf("no valid rtcm msgs\n");
        return 0;
    }
    int n = i;
    uint16_t offset = 0;
    uint16_t length = 0;

    for(i = 0; i < n; i++){

        msg[i].encode(buf+offset,length);
        offset += length;

    }
    len = offset;


    return 1;
}

bool RTCM3_UBLOX::encode_UBX(uint8_t *buf, uint16_t &len){
    int i = msg_ready(UBX);
    if(i == 0){
        printf("no valid ubx msgs\n");
        return 0;
    }
    int n = i;
    uint16_t offset = 0;
    uint16_t length = 0;
    for(i = 0; i < n; i++){

        ubx[i].encode(buf+offset,length);
        offset += length;

    }
    len = offset;
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
    if(rtcm_msg_pointer > MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES){
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