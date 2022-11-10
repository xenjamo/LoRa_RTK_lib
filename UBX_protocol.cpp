
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


// decodes each message according to interface discription:
// (https://content.u-blox.com/sites/default/files/u-blox_ZED-F9H_InterfaceDescription_%28UBX-19030118%29.pdf) (last checked 09.11.2022)
bool UBX_MSG::ubx2string(char *buf, uint16_t &len){

    if(!isvalid) return 0;
    uint16_t offset = 0;
    uint16_t l = 0;
    
    char ubx_nav[] = "UBX-NAV";

    uint32_t utemp32[5];
    int32_t temp32[5];
    uint16_t utemp16[5];
    double tempf[5];

    char temp[400];
    switch(_class){
        case(0x01):
        insert2array(buf, ubx_nav, sizeof(ubx_nav), offset);
        switch(id){
            case(0x3b):
            l = sprintf(temp, "-SVIN;")+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "ver;%02x;", data[0])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24);
            utemp32[1] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            l = sprintf(temp, "iTOW [ms];%d;dur;%d;",utemp32[0],utemp32[1])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            temp32[1] = (uint32_t)data[16] | ((uint32_t)data[17] << 8) | ((uint32_t)data[18] << 16) | ((uint32_t)data[19] << 24);
            temp32[2] = (uint32_t)data[20] | ((uint32_t)data[21] << 8) | ((uint32_t)data[22] << 16) | ((uint32_t)data[23] << 24);
            l = sprintf(temp, "meanX,Y,Z [cm];%d;%d;%d;",temp32[0],temp32[1],temp32[2])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "meanX,Y,Z HP [0.1 mm];%d;%d;%d;",(int8_t)data[24],(int8_t)data[25],(int8_t)data[26])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[28] | ((uint32_t)data[29] << 8) | ((uint32_t)data[30] << 16) | ((uint32_t)data[31] << 24);
            l = sprintf(temp, "meanAcc HP [0.1 mm];%d;",utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[32] | ((uint32_t)data[33] << 8) | ((uint32_t)data[34] << 16) | ((uint32_t)data[35] << 24);
            l = sprintf(temp, "obs;%d;valid;%d;active;%d;",utemp32[0],data[36],data[37])+1;
            insert2array(buf, temp, l, offset);

            break;
            case(0x03):
            l = sprintf(temp, "-STATUS;")+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
            l = sprintf(temp, "iTOW [ms];%d;",utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "gpsFix;0x%x;",data[4])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "gpsFixOk;%d;",data[5] & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "diffSol;%d;",(data[5] >> 1) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "wknSet;%d;",(data[5] >> 2) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "towSet;%d;",(data[5] >> 3) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "diffCorr;%d;",(data[6] >> 0) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "carrSolValid;%d;",(data[6] >> 1) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "mapMatching;%d;",(data[6] >> 6) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "psmState;%d;",(data[7] >> 0) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "spoofDetState;%d;",(data[7] >> 2) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "carrSoln;%d;",(data[7] >> 6) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            utemp32[1] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            l = sprintf(temp, "ttff;%d;msss;%d;",utemp32[0],utemp32[1])+1;
            insert2array(buf, temp, l, offset);

            break;
            case(0x02):
            l = sprintf(temp, "-POSLLH;")+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[ 3] << 24);
            l = sprintf(temp, "iTOW [ms];%d;",utemp32[0])+1;
            temp32[0] = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[ 6] << 16) | ((uint32_t)data[ 7] << 24);
            temp32[1] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            tempf[0] = (double)temp32[0] / 10000000;
            tempf[1] = (double)temp32[1] / 10000000;
            l = sprintf(temp, "lon [deg];%3.8f;lat [deg];%3.8f;",tempf[0],tempf[1])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            utemp32[0] = (uint32_t)data[16] | ((uint32_t)data[17] << 8) | ((uint32_t)data[18] << 16) | ((uint32_t)data[19] << 24);
            l = sprintf(temp, "heigth [mm];%d;hMSL [mm];%d;",temp32[0],utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[20] | ((uint32_t)data[21] << 8) | ((uint32_t)data[22] << 16) | ((uint32_t)data[23] << 24);
            utemp32[1] = (uint32_t)data[24] | ((uint32_t)data[25] << 8) | ((uint32_t)data[26] << 16) | ((uint32_t)data[27] << 24);
            l = sprintf(temp, "hAcc [mm];%d;vAcc [mm];%d;",utemp32[0],utemp32[1])+1;
            insert2array(buf, temp, l, offset);

            break;
            case(0x3c):
            l = sprintf(temp, "-RELPOSNED;")+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "ver;%02x;", data[0])+1;
            insert2array(buf, temp, l, offset);
            utemp16[0] = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
            utemp32[0] = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24);
            l = sprintf(temp, "revID;%d;iTOW [ms];%d;",utemp16[0],utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            temp32[1] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            temp32[2] = (uint32_t)data[16] | ((uint32_t)data[17] << 8) | ((uint32_t)data[18] << 16) | ((uint32_t)data[19] << 24);
            l = sprintf(temp, "relposN,E,D [cm];%d;%d;%d;",temp32[0],temp32[1],temp32[2])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[20] | ((uint32_t)data[21] << 8) | ((uint32_t)data[22] << 16) | ((uint32_t)data[23] << 24);
            temp32[0] = (uint32_t)data[24] | ((uint32_t)data[25] << 8) | ((uint32_t)data[26] << 16) | ((uint32_t)data[27] << 24);
            tempf[0] = (double)temp32[0] / 100000;
            l = sprintf(temp, "dist [cm];%d;heading [deg];%3.5f;",utemp32[0],tempf[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "relposN,E,D HP [0.1 mm];%d;%d;%d;",(int8_t)data[32],(int8_t)data[33],(int8_t)data[34])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "dist HP [0.1 mm];%d;",(int8_t)data[35])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[36] | ((uint32_t)data[37] << 8) | ((uint32_t)data[38] << 16) | ((uint32_t)data[39] << 24);
            utemp32[1] = (uint32_t)data[40] | ((uint32_t)data[41] << 8) | ((uint32_t)data[42] << 16) | ((uint32_t)data[43] << 24);
            utemp32[2] = (uint32_t)data[44] | ((uint32_t)data[45] << 8) | ((uint32_t)data[46] << 16) | ((uint32_t)data[47] << 24);
            l = sprintf(temp, "accN,E,D [0.1 mm];%d;%d;%d;",utemp32[0],utemp32[1],utemp32[2])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[48] | ((uint32_t)data[49] << 8) | ((uint32_t)data[50] << 16) | ((uint32_t)data[51] << 24);
            temp32[0] = (uint32_t)data[52] | ((uint32_t)data[53] << 8) | ((uint32_t)data[54] << 16) | ((uint32_t)data[55] << 24);
            tempf[0] = (double)temp32[0] / 100000;
            l = sprintf(temp, "accDist [0.1 mm];%d;accHead [deg];%f;",utemp32[0],tempf[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "gnssFixOk;%d;",(data[60] >> 0) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "diffSoln;%d;",(data[60] >> 1) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "relPosValid;%d;",(data[60] >> 2) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "carrSoln;%d;",(data[60] >> 3) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "isMoving;%d;",(data[60] >> 5) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "refPosMiss;%d;",(data[60] >> 6) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "refOpsMiss;%d;",(data[60] >> 7) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "relPosHeadValid;%d;",(data[61] >> 0) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "relPosNormalized;%d;",(data[61] >> 1) & 0x01)+1;
            insert2array(buf, temp, l, offset);



            break;
            default:
            printf("id 0x%x not supported\n", id);
            return false;
            break;
        }


        break;
        default:
        printf("class 0x%x not supported\n", _class);
        return false;
        break;
    }
    len = offset;
    clear();
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