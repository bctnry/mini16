#include "fs.h"
#include "../util/visual_hint.h"

char * DMRT = (char*)0x1600;

char fs_list_dir(char drive_n, const char* ls) {
    return 0;    
}

void fs_mount(char drive_n, char type) {
    DMRT[drive_n] = type;
}

char fs_gettype(char drive_n) {
    return DMRT[drive_n];
}

char far* fs_getcwd() {
    return (char far*)0x00000500;
}


