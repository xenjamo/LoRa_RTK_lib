


#include "SD_interface.h"





SDCARD::SDCARD(int l) : fs("sd"), _sd(PC_12, PC_11, PC_10, PD_2){
    
    init_success = 0;//nothing special here

}


bool SDCARD::init(){
    if (0 != _sd.init()) {
        printf("Init failed \n");
        return 0;
    }
    if (0 != _sd.frequency(12000000)) {
        printf("Error setting frequency \n");
    }

    fflush(stdout);

    if(fs.mount(&_sd) != 0){
        printf("mount failed\n");
        return 0;
    }
    fflush(stdout);
    mkdir("/sd/data",0777);
    
    int i = 0;
    
    while(1){
        i++;
        sprintf(path, "/sd/data/%03i.csv", i);
        fp = fopen(path, "r");
        if(fp == NULL){
            fp = fopen(path, "w");
            fprintf(fp,"sep=;\nRTK_GPS_DATA;%s\n",path);

            printf("working in file: %s\n",path);
            break;
        } else {
            fclose(fp);
        }
        if(i >= 999){
            printf("maximum files reached\n");
            fclose(fp);
            return 0;
        }
    }
    //fclose(fp);
    init_success = true;
    return 1;
}


bool SDCARD::write2sd(char *data, int l){
    if(!init_success) return 0;

    //FILE* fp = fopen(path, "a");
    if(fp == NULL){
        return 0;
    }
    fprintf(fp, "%s", data);
    //fclose(fp);

    return 1;
}

bool SDCARD::writeln(){
    if(!init_success) return 0;

    char tmp[] = "\n";
    if(!write2sd(tmp,sizeof(tmp))){
        return 0;
    }
    return 1;
}