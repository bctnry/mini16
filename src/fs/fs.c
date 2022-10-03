#include "../const.h"
#include "fs.h"
#include "../util/visual_hint.h"


char fs_list_dir(char drive_n, const char* ls) {
    return 0;    
}

void fs_mount(char drive_n, char type) {
    DMRT_FAR[drive_n] = type;
}

char fs_gettype(char drive_n) {
    return DMRT_FAR[drive_n];
}

char far* fs_getcwd() {
    return CWD_FAR;
}


