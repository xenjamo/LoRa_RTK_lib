


#include "SD_interface.h"





SDCARD::SDCARD(int l) : fs("sd"), _sd(PC_12, PC_11, PC_10, PD_2){
    
    //nothing special here

}


bool SDCARD::init(){
    if (0 != _sd.init()) {
        printf("Init failed \n");
        return 0;
    }
    fflush(stdout);

    if(fs.mount(&_sd) != 0){
        printf("mount failed\n");
        return 0;
    }
    fflush(stdout);
    mkdir("/sd/data",0777);
    
    int i = 0;
    FILE* fp;
    while(1){
        i++;
        sprintf(path, "/sd/data/%02i.csv", i);
        fp = fopen(path, "r");
        if(fp == NULL){
            fp = fopen(path, "w");
            fprintf(fp,"RTK_GPS_DATA\n");

            printf("working in file: %s",path);
            break;
        } else {
            fclose(fp);
        }
        if(i >= 99){
            printf("maximum files reached\n");
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);

    return 1;
}


bool SDCARD::write2sd(char *data, int l){
    FILE* fp = fopen(path, "a");
    if(fp == NULL){
        return 0;
    }
    fprintf(fp, "%s", data);
    fclose(fp);

    return 1;
}

bool SDCARD::writeln(){
    char tmp[] = "\n";
    if(!write2sd(tmp,sizeof(tmp))){
        return 0;
    }
    return 1;
}