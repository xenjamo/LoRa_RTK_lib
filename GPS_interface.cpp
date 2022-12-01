

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
    rtcm_msg = (uint8_t*) malloc(MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES); //allocate memory in heap (stacksize to small)
    uart_buf = (uint8_t*) malloc(MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES);
    
    clear_buf(rtcm_msg, MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES); //set all to 0 else it will cause issues
    clear_buf(uart_buf, MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES);
}




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
    clearUART();
    return 1;
}


bool RTCM3_UBLOX::reset_pos(){
    itow = 0;
    lon = 0.0f;
    lat = 0.0f;
    height = 0.0f;
    
    rel_x = 0.0f;
    rel_y = 0.0f;
    rel_z = 0.0f;
    
    rtk_stat = 0;
    hAcc = 0;
    vAcc = 0;
    return true;
}


//returns 0 when no activity 1 when data is arriving (theoreticly)
bool RTCM3_UBLOX::msg_activity(){
    //led1 = !led1;

    if(t.elapsed_time() > 20ms){
        //if this triggers message should be recieved
        //please optimize in the future
        isactive = 0;
        return 0;
    }
    return 1;
}

bool RTCM3_UBLOX::data_ready(){
    if((msg_pos == MSG_DATA) & !msg_activity()){
        memcpy(rtcm_msg, uart_buf, uart_buf_pointer);
        rtcm_msg_length = uart_buf_pointer;
        clearUART();
        return 1;
    }
    return 0;
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

bool RTCM3_UBLOX::printMsgTypes(){
    
    uint8_t n = msg_ready(RTCM);
    if(n == 0){
        //printf("no rtcm msg\n");
        return false;
    }

    printf("Msg types are:");

    for(int i = 0; i < n; i++){
        printf(" %d,",msg[i].type);
    }
    printf("\n");
    return true;
    
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
    for(int i = 0; i < MAXIMUM_UBX_MESSAGES; i++){
        ubx[i].clear();
    }
    clear_buf(rtcm_msg, MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES);
    reached_max_msg = 0;
    rtcm_msg_pointer = 0;
    rtcm_msg_length = 0;
    return 1;
}

bool RTCM3_UBLOX::clearUART(){
    clear_buf(uart_buf, MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES);
    msg_pos = MSG_IDLE;
    uart_buf_pointer = 0;
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
                msg[n].data[i] = rtcm_msg[p_offset + 3 + i];
            }
            msg[n].type = (((uint16_t)rtcm_msg[p_offset + 3] << 8) + rtcm_msg[p_offset + 4])  >> 4;
            msg[n].crc = ((uint32_t)rtcm_msg[p_offset + 3 + msg[n].length] << 16)
                        +((uint32_t)rtcm_msg[p_offset + 3 + msg[n].length + 1] <<  8)
                        +((uint32_t)rtcm_msg[p_offset + 3 + msg[n].length + 2]);
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
            ubx[k].length = ((uint16_t)rtcm_msg[p_offset+5] << 8) + rtcm_msg[p_offset+4];
            ubx[k].data = (uint8_t*)calloc(ubx[k].length, 1);
            for(int i = 0; i < ubx[k].length; i++){
                ubx[k].data[i] = rtcm_msg[p_offset+6+i];
            }
            
            ubx[k].ch_[0] = rtcm_msg[p_offset+6+ubx[k].length];
            ubx[k].ch_[1] = rtcm_msg[p_offset+6+ubx[k].length+1];
            p_offset = p_offset + 8 + ubx[k].length;
            ubx[k].isvalid = true;
            k++;


        }else if(first_byte == '$'){
            printf("a NMEA msg\n");
            return 0;

        }else if(first_byte == 0){
            rtcm_msg_length = p_offset;
            loop = 0;
        }else{
            rtcm_msg_length = p_offset;
            loop = 0;
            printf("preamble wrong: 0x%02x\n", first_byte);
            return 0;
        }
        
        
        

    }

    decodeUBX(); //decode diffrent ubx messages and store its values in this object


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
    uart_buf[uart_buf_pointer] = c; //save read message
    uart_buf_pointer++;             //advance pointer
    msg_pos = MSG_DATA;             //indicate we are receiving data


    //if pointer gets out of bounds it creates errors so here we set a flag to indicate the buffer is full or faulty
    if(uart_buf_pointer > MAXIMUM_RTCM_MESSAGE_LENGTH*MAXIMUM_RTCM_MESSAGES){
        msg_pos = MSG_ERR;
    }
    
    return;

}



bool RTCM3_UBLOX::decodeUBX(){
    int i = 0;
    while(ubx[i].isvalid){
        switch(ubx[i]._class){
            case(0x01):
            //
            switch(ubx[i].id){
                case(0x14):{
                    itow = (uint32_t)ubx[i].data[4] | ((uint32_t)ubx[i].data[5] << 8) | ((uint32_t)ubx[i].data[6] << 16) | ((uint32_t)ubx[i].data[ 7] << 24);
                    // LL LP
                    int32_t temp32_1 = (uint32_t)ubx[i].data[ 8] | ((uint32_t)ubx[i].data[ 9] << 8) | ((uint32_t)ubx[i].data[10] << 16) | ((uint32_t)ubx[i].data[11] << 24); //lon
                    int32_t temp32_2 = (uint32_t)ubx[i].data[12] | ((uint32_t)ubx[i].data[13] << 8) | ((uint32_t)ubx[i].data[14] << 16) | ((uint32_t)ubx[i].data[15] << 24); //lat
                    lon = (double)temp32_1 / 10000000;
                    lat = (double)temp32_2 / 10000000;
                    // height LP
                    temp32_1 = (uint32_t)ubx[i].data[16] | ((uint32_t)ubx[i].data[17] << 8) | ((uint32_t)ubx[i].data[18] << 16) | ((uint32_t)ubx[i].data[19] << 24); //height
                    height = (double)temp32_1 / 1000;

                    // LL HP
                    lon = lon + ((double)(int8_t)ubx[i].data[24]) / 1000000000;
                    lat = lat + ((double)(int8_t)ubx[i].data[25]) / 1000000000;
                    // height HP
                    height = height + (double)(int8_t)ubx[i].data[26] / 10000;

                    hAcc = (double)(int32_t)((uint32_t)ubx[i].data[28] | ((uint32_t)ubx[i].data[29] << 8) | ((uint32_t)ubx[i].data[30] << 16) | ((uint32_t)ubx[i].data[31] << 24)) / 10; //hAcc
                    vAcc = (double)(int32_t)((uint32_t)ubx[i].data[32] | ((uint32_t)ubx[i].data[33] << 8) | ((uint32_t)ubx[i].data[34] << 16) | ((uint32_t)ubx[i].data[35] << 24)) / 10; //vAcc
                    break;
                }
                case(0x3c):{
                    itow = (uint32_t)ubx[i].data[4] | ((uint32_t)ubx[i].data[5] << 8) | ((uint32_t)ubx[i].data[6] << 16) | ((uint32_t)ubx[i].data[7] << 24); //itow
                    
                    rel_x = (double)(int32_t)((uint32_t)ubx[i].data[ 8] | ((uint32_t)ubx[i].data[ 9] << 8) | ((uint32_t)ubx[i].data[10] << 16) | ((uint32_t)ubx[i].data[11] << 24))*10; //relpos N
                    rel_y = (double)(int32_t)((uint32_t)ubx[i].data[12] | ((uint32_t)ubx[i].data[13] << 8) | ((uint32_t)ubx[i].data[14] << 16) | ((uint32_t)ubx[i].data[15] << 24))*10; //relpos E
                    rel_z = (double)(int32_t)((uint32_t)ubx[i].data[16] | ((uint32_t)ubx[i].data[17] << 8) | ((uint32_t)ubx[i].data[18] << 16) | ((uint32_t)ubx[i].data[19] << 24))*10; //relpos D

                    rel_x = rel_x + (double)(int8_t)ubx[i].data[32] / 10;
                    rel_y = rel_y + (double)(int8_t)ubx[i].data[33] / 10;
                    rel_z = rel_z + (double)(int8_t)ubx[i].data[34] / 10;

                    rtk_stat = (ubx[i].data[60] >> 3) & 0x03;
                    break;
                }

                case(0x03):{
                    //nothing here
                    break;
                }

                default:
                printf("id 0x%x not supported",ubx[i].id);
                break;
            }
            break;
            default:
            printf("class 0x%x not supported\n", ubx[i]._class);
            return 0;
            break;
        }
        i++;
    }

    return 1;
}


//set all values of an array to 0
void clear_buf(uint8_t *buf, int length){
    for(int i = 0; i<length; i++){
        buf[i] = 0;
    }

}