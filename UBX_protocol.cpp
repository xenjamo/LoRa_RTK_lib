
#include "UBX_protocol.h"

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
    len = length + 8;
    buf[0] = header >> 8;
    buf[1] = header;

    buf[2] = _class;
    buf[3] = id;

    buf[5] = length >> 8;
    buf[4] = length;
    for(int i = 0; i < length; i++){
        buf[6+i] = data[i];
    }
    buf[6+length] = ch_[0];
    buf[6+length +1] = ch_[1];
    clear();
}

uint16_t UBX_MSG::getlength(){
    return length + 8; //not entirely correct some UBX msgs can have diffrent structures please keep that in mind
}

bool UBX_MSG::clear(){
    std::fill_n(data, length, 0);
    header = 0;
    _class = 0;
    id = 0;
    length = 0;
    isvalid = 0;
    ch_[0] = 0;
    ch_[1] = 0;
    return 1;
}
bool UBX_MSG::ubx2string(char *buf, uint16_t &len){
    uint16_t offset = 0;
    uint16_t l = 0;
    char ubx_nav[] = "UBX-NAV";
    char svin[] = "-SVIN;";
    uint32_t temp32[5];
    uint16_t temp16[5];
    double tempf[5];

    char temp[200];
    switch(_class){
        case(0x01):
        insert2array(buf, ubx_nav, sizeof(ubx_nav), offset);
        switch(id){
            case(0x3b):
            insert2array(buf, svin, sizeof(svin), offset);
            l = sprintf(temp, "ver = ;%02x;", data[0])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[4] + ((uint32_t)data[5] << 8) + ((uint32_t)data[6] << 16) + ((uint32_t)data[7] << 24);
            temp32[1] = (uint32_t)data[8] + ((uint32_t)data[9] << 8) + ((uint32_t)data[10] << 16) + ((uint32_t)data[11] << 24);
            l = sprintf(temp, "iTOW [ms];%d;dur;%d;",temp32[0],temp32[1])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[12] + ((uint32_t)data[13] << 8) + ((uint32_t)data[14] << 16) + ((uint32_t)data[15] << 24);
            temp32[1] = (uint32_t)data[16] + ((uint32_t)data[17] << 8) + ((uint32_t)data[18] << 16) + ((uint32_t)data[18] << 24);
            temp32[2] = (uint32_t)data[20] + ((uint32_t)data[21] << 8) + ((uint32_t)data[22] << 16) + ((uint32_t)data[23] << 24);
            l = sprintf(temp, "meanX,Y,Z [cm];%d;%d;%d;",temp32[0],temp32[1],temp32[2])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[24] + ((uint32_t)data[25] << 8) + ((uint32_t)data[26] << 16) + ((uint32_t)data[27] << 24);
            temp32[1] = (uint32_t)data[28] + ((uint32_t)data[29] << 8) + ((uint32_t)data[30] << 16) + ((uint32_t)data[31] << 24);
            temp32[2] = (uint32_t)data[32] + ((uint32_t)data[33] << 8) + ((uint32_t)data[34] << 16) + ((uint32_t)data[35] << 24);
            l = sprintf(temp, "meanX,Y,Z (hp) [0.1 mm];%d;%d;%d;",temp32[0],temp32[1],temp32[2])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[28] + ((uint32_t)data[29] << 8) + ((uint32_t)data[30] << 16) + ((uint32_t)data[31] << 24);
            l = sprintf(temp, "meanAcc (hp) [0.1 mm];%d",temp32[0])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[32] + ((uint32_t)data[33] << 8) + ((uint32_t)data[34] << 16) + ((uint32_t)data[35] << 24);
            l = sprintf(temp, "obs;%d;valid;%d;active;%d;",temp32[0],data[36],data[37])+1;
            insert2array(buf, temp, l, offset);

            break;
            default:
            break;
        }


        break;
        default:
        printf("class 0x%x not supported\n", _class);
        return false;
        break;
    }
    return true;
}

void UBX_MSG::insertsemi(char* dst, uint16_t &offset){
    char semi[] = ";";
    insert2array(dst,semi,sizeof(semi),offset);
}

void UBX_MSG::insert2array(char* dst, char* src, uint16_t len, uint16_t &offset){
    memcpy(dst+offset, src, len);
    offset += len-1;
}

////// end of UBX msg block//////////