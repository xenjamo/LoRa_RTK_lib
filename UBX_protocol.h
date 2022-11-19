
#include <cstdint>
#include <string>



class UBX_MSG{

    public:
    UBX_MSG();
    uint16_t header;
    uint8_t _class;
    uint8_t id;
    uint16_t length;
    uint8_t *data;
    bool isvalid;
    uint8_t ch_[2];
    void encode(uint8_t *buf, uint16_t &len);
    uint16_t getlength();
    bool ubxHeader(char* buf, uint16_t &len);
    bool ubx2string(char* buf, uint16_t &len);
    bool clear();
    private:
    void insertsemi(char* dst, uint16_t &offset);
    void insert2array(char* dst, char* src, uint16_t len, uint16_t &offset);

};