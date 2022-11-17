


#include "SD_interface.h"





SDCARD::SDCARD() : fs(MOUNT_PATH), sd(PC_12, PC_11, PC_10, PD_2){
    
    //nothing special here

}


bool SDCARD::init(){
    if (0 != sd.init()) {
        printf("Init failed \n");
        return 0;
    }
    fflush(stdout);

    if(fs.mount(&sd) != 0){
        printf("mount failed\n");
        return 0;
    }
    fflush(stdout);
    int i = 0;
    char buf[] = "RTK_GPS_data\n";
    FILE* fp;
    while(1){
        i++;
        sprintf(path, "/sd/data/%02i.csv", i);
        fp = fopen(path, "r");
        if(fp == NULL){
            fclose(fp);
            write2sd(buf, sizeof(buf));
            break;
        } else {
            fclose(fp);
        }
        if(i >= 99){
            printf("maximum files reached\n");
            return 0;
        }
    }

    return 1;
}


bool SDCARD::write2sd(char *data, int l){
    FILE* fp = fopen(path, "a");
    if(fp == NULL){
        return 0;
    }
    fprintf(fp, "%s", data);

    return 1;
}

bool SDCARD::writeln(){
    char tmp[] = "\n";
    if(!write2sd(tmp,sizeof(tmp))){
        return 0;
    }
    return 1;
}