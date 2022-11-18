#include "mbed.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"

#define MOUNT_PATH "sd"

class SDCARD{
    public:
    SDCARD(int l);

    bool init();
    bool write2sd(char* data, int l);
    bool writeln();

    private:
    char path[30];
    FATFileSystem fs;
    SDBlockDevice _sd;


};