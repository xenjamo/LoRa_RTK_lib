
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
std::string UBX_MSG::tostring(){
    std::string s;
    switch(_class){
        case(0x01):
        s.append("UBX-NAV");
        switch(id){
            case(0x25):
            s.append("-SVIN;");
            
            break;
            default:
            break;
        }


        break;
        default:
        printf("class 0x%x not supported\n", _class);
        return NULL;
        break;
    }
    return s;
}

////// end of UBX msg block//////////