#ifndef SD_INTERFACE_H_
#define SD_INTERFACE_H_


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
    bool init_success;
    char path[30];
    FATFileSystem fs;
    SDBlockDevice _sd;
    FILE* fp;


};

#endif