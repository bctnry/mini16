#include "../const.h"
#include "cwd.h"

#define CWD_LEN_FAR ((char far*)0x00000600)
// INVARIANT: &CWD_FAR[CWD_LEN_FAR] should points you at the end of the string.

char read_nibble_ch(char x) {
    char res = x >= 'a'? (x - 'a' + 0x0a) : x >= 'A'? (x - 'A' + 0x0a) : x - '0';
    return res;
}

char cwd_goto(const char* str) {
    return 0;
}



char cwd_get_drive_n() {
    return (read_nibble_ch(CWD_FAR[1])<<4)|read_nibble_ch(CWD_FAR[2]);
}

void cwd_goto_parent() {
    unsigned short len = *CWD_LEN_FAR;
    unsigned short i = len-1;
    while (i > 0 && CWD_FAR[i] != '\\') {
        i--;
    }
    if (i - 1 > 0) {
        i -= 1;
        CWD_FAR[i] = '\0';
        *CWD_LEN_FAR = (unsigned short)i+1;
    }
}

void cwd_set_drive_n(char drive_n) {
    char z;
    z = drive_n >> 4;
    CWD_FAR[1] = z >= 0x0a? z + 'A' - 0x0a : z + 0x30;
    drive_n %= 0xf;
    CWD_FAR[2] = drive_n >= 0x0a? drive_n + 'A' - 0x0a : drive_n + 0x30;
}
