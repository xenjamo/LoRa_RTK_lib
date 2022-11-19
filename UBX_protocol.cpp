
#include "UBX_protocol.h"
#include <cstdint>

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

bool UBX_MSG::ubxHeader(char *buf, uint16_t &len){
    static bool called = 0;
    
    if(called) return 0;

    uint16_t offset = 0;
    uint16_t l = 0;
    char temp[400];

    switch(_class){
        case(0x01):
        switch(id){
            case(0x3b):
            l = sprintf(temp, "name;ver;iTOW [ms];dur;meanX [cm];meanY [cm];meanZ [cm];meanXHP [0.1 mm];meanYHP [0.1 mm];meanZHP [0.1 mm];meanAcc HP [0.1 mm];obs;valid;active;")+1;
            insert2array(buf,temp,l,offset);
            break;
            case(0x03):
            l = sprintf(temp, "name;iTOW [ms];gpsFix;gpsFixOk;diffSol;wknSet;towSet;diffCorr;carrSolValid;mapMatching;psmState;spoofDetState;carrSoln;ttff;msss;")+1;
            insert2array(buf,temp,l,offset);
            break;
            case(0x02):
            l = sprintf(temp, "name;iTOW [ms];lon [deg];lat [deg];heigth [mm];hMSL [mm];hAcc [mm];vAcc [mm];")+1;
            insert2array(buf,temp,l,offset);
            break;
            case(0x014):
            l = sprintf(temp, "name;ver;invalidLlh;iTOW [ms];lon [deg];lat [deg];heigth [mm];hMSL [mm];lon HP [deg];lat HP [deg];height HP [0.1 mm];hMSL HP [0.1 mm];hAcc [0.1 mm];vAcc [0.1 mm];")+1;
            insert2array(buf, temp, l, offset);
            break;
            case(0x3c):
            l = sprintf(temp, "name;ver;revID;iTOW [ms];relposN[cm];relposE [cm];relposD [cm];dist [cm];heading [deg];relposNHP [0.1 mm];relposEHP [0.1 mm];relposDHP [0.1 mm];dist HP [0.1 mm];accN [0.1 mm];accE [0.1 mm];accD [0.1 mm];accDist [0.1 mm];accHead [deg];gnssFixOk;diffSoln;relPosValid;carrSoln;isMoving;refPosMiss;refOpsMiss;relPosHeadValid;relPosNormalized;")+1;
            insert2array(buf,temp,l,offset);
            break;
        }
        break;
    }
    called = true;
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
            l = sprintf(temp, "%02x;", data[0])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24);
            utemp32[1] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            l = sprintf(temp, "%u;%u;",utemp32[0],utemp32[1])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            temp32[1] = (uint32_t)data[16] | ((uint32_t)data[17] << 8) | ((uint32_t)data[18] << 16) | ((uint32_t)data[19] << 24);
            temp32[2] = (uint32_t)data[20] | ((uint32_t)data[21] << 8) | ((uint32_t)data[22] << 16) | ((uint32_t)data[23] << 24);
            l = sprintf(temp, "%d;%d;%d;",temp32[0],temp32[1],temp32[2])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;%d;%d;",(int8_t)data[24],(int8_t)data[25],(int8_t)data[26])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[28] | ((uint32_t)data[29] << 8) | ((uint32_t)data[30] << 16) | ((uint32_t)data[31] << 24);
            l = sprintf(temp, "%u;",utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[32] | ((uint32_t)data[33] << 8) | ((uint32_t)data[34] << 16) | ((uint32_t)data[35] << 24);
            l = sprintf(temp, "%u;%d;%d;",utemp32[0],data[36],data[37])+1;
            insert2array(buf, temp, l, offset);

            break;
            case(0x03):
            l = sprintf(temp, "-STATUS;")+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
            l = sprintf(temp, "%u;",utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "0x%x;",data[4])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",data[5] & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[5] >> 1) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[5] >> 2) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[5] >> 3) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[6] >> 0) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[6] >> 1) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[6] >> 6) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[7] >> 0) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[7] >> 2) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[7] >> 6) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            utemp32[1] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            l = sprintf(temp, "%u;%u;",utemp32[0],utemp32[1])+1;
            insert2array(buf, temp, l, offset);

            break;
            case(0x02):
            l = sprintf(temp, "-POSLLH;")+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[ 3] << 24);
            l = sprintf(temp, "%u;",utemp32[0])+1;
            temp32[0] = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[ 6] << 16) | ((uint32_t)data[ 7] << 24);
            temp32[1] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            tempf[0] = (double)temp32[0] / 10000000;
            tempf[1] = (double)temp32[1] / 10000000;
            l = sprintf(temp, "%3.7f;%3.7f;",tempf[0],tempf[1])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            utemp32[0] = (uint32_t)data[16] | ((uint32_t)data[17] << 8) | ((uint32_t)data[18] << 16) | ((uint32_t)data[19] << 24);
            l = sprintf(temp, "%d;%d;",temp32[0],utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[20] | ((uint32_t)data[21] << 8) | ((uint32_t)data[22] << 16) | ((uint32_t)data[23] << 24);
            utemp32[1] = (uint32_t)data[24] | ((uint32_t)data[25] << 8) | ((uint32_t)data[26] << 16) | ((uint32_t)data[27] << 24);
            l = sprintf(temp, "%u;%u;",utemp32[0],utemp32[1])+1;
            insert2array(buf, temp, l, offset);

            break;
            case(0x14):
            l = sprintf(temp, "-HPPOSLLH;")+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "0x%02x;", data[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;", data[3] & 0x01)+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) | ((uint32_t)data[ 7] << 24);
            l = sprintf(temp, "%d;",utemp32[0])+1;
            temp32[0] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            temp32[1] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            tempf[0] = (double)temp32[0] / 10000000;
            tempf[1] = (double)temp32[1] / 10000000;
            l = sprintf(temp, "%3.7f;%3.7f;",tempf[0],tempf[1])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[16] | ((uint32_t)data[17] << 8) | ((uint32_t)data[18] << 16) | ((uint32_t)data[19] << 24);
            utemp32[0] = (uint32_t)data[20] | ((uint32_t)data[21] << 8) | ((uint32_t)data[22] << 16) | ((uint32_t)data[23] << 24);
            l = sprintf(temp, "%d;%u;",temp32[0],utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%1.9f;%1.9f;",((double)(int8_t)data[24]) / 1000000000,((double)(int8_t)data[25]) / 1000000000)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;%d;",(int8_t)data[26],(int8_t)data[27])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[28] | ((uint32_t)data[29] << 8) | ((uint32_t)data[30] << 16) | ((uint32_t)data[31] << 24);
            utemp32[1] = (uint32_t)data[32] | ((uint32_t)data[33] << 8) | ((uint32_t)data[34] << 16) | ((uint32_t)data[35] << 24);
            l = sprintf(temp, "%u;%u;",utemp32[0],utemp32[1])+1;
            insert2array(buf, temp, l, offset);

            break;
            case(0x3c):
            l = sprintf(temp, "-RELPOSNED;")+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "0x%02x;", data[0])+1;
            insert2array(buf, temp, l, offset);
            utemp16[0] = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
            utemp32[0] = (uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24);
            l = sprintf(temp, "%u;%u;",utemp16[0],utemp32[0])+1;
            insert2array(buf, temp, l, offset);
            temp32[0] = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
            temp32[1] = (uint32_t)data[12] | ((uint32_t)data[13] << 8) | ((uint32_t)data[14] << 16) | ((uint32_t)data[15] << 24);
            temp32[2] = (uint32_t)data[16] | ((uint32_t)data[17] << 8) | ((uint32_t)data[18] << 16) | ((uint32_t)data[19] << 24);
            l = sprintf(temp, "%d;%d;%d;",temp32[0],temp32[1],temp32[2])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[20] | ((uint32_t)data[21] << 8) | ((uint32_t)data[22] << 16) | ((uint32_t)data[23] << 24);
            temp32[0] = (uint32_t)data[24] | ((uint32_t)data[25] << 8) | ((uint32_t)data[26] << 16) | ((uint32_t)data[27] << 24);
            tempf[0] = (double)temp32[0] / 100000;
            l = sprintf(temp, "%u;%3.5f;",utemp32[0],tempf[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;%d;%d;",(int8_t)data[32],(int8_t)data[33],(int8_t)data[34])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(int8_t)data[35])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[36] | ((uint32_t)data[37] << 8) | ((uint32_t)data[38] << 16) | ((uint32_t)data[39] << 24);
            utemp32[1] = (uint32_t)data[40] | ((uint32_t)data[41] << 8) | ((uint32_t)data[42] << 16) | ((uint32_t)data[43] << 24);
            utemp32[2] = (uint32_t)data[44] | ((uint32_t)data[45] << 8) | ((uint32_t)data[46] << 16) | ((uint32_t)data[47] << 24);
            l = sprintf(temp, "%u;%u;%u;",utemp32[0],utemp32[1],utemp32[2])+1;
            insert2array(buf, temp, l, offset);
            utemp32[0] = (uint32_t)data[48] | ((uint32_t)data[49] << 8) | ((uint32_t)data[50] << 16) | ((uint32_t)data[51] << 24);
            temp32[0] = (uint32_t)data[52] | ((uint32_t)data[53] << 8) | ((uint32_t)data[54] << 16) | ((uint32_t)data[55] << 24);
            tempf[0] = (double)temp32[0] / 100000;
            l = sprintf(temp, "%u;%f;",utemp32[0],tempf[0])+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[60] >> 0) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[60] >> 1) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[60] >> 2) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[60] >> 3) & 0x03)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[60] >> 5) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[60] >> 6) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[60] >> 7) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[61] >> 0) & 0x01)+1;
            insert2array(buf, temp, l, offset);
            l = sprintf(temp, "%d;",(data[61] >> 1) & 0x01)+1;
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