#include "../const.h"
#include "cwd.h"
#include "../cstdlib/string.h"
#include "../util/debug.h"
#include "../term/term.h"

#define CWD_LEN_FAR ((char far*)0x00000600)
// INVARIANT: &CWD_FAR[CWD_LEN_FAR] should points you at the end of the string.

char read_nibble_ch(char x) {
    char res = x >= 'a'? (x - 'a' + 0x0a) : x >= 'A'? (x - 'A' + 0x0a) : x - '0';
    return res;
}

void cwd_goto(const char* str) {
    unsigned short i = 0;
    unsigned short i_ = 0;
    unsigned short j = 0;
    char z;
    char far* CWD_END = &CWD_FAR[*CWD_LEN_FAR];
    while (str[i]) {
        while (str[i] && str[i] != '/') { i++; }
        if (i_ == i) {
            i_ = i = i+1;
            continue;
        } else if (i-i_==2 && str[i_]=='.' && str[i_+1]=='.') {
            cwd_goto_parent();
            CWD_END = &CWD_FAR[*CWD_LEN_FAR];
        } else {
            j = i_;
            while (j < i) {
                *CWD_END = str[j];
                CWD_END++;
                j++;
            }
            *CWD_END = '/'; CWD_END++;
            *CWD_LEN_FAR = *CWD_LEN_FAR + i - i_ + 1;
        }
        // NOTE: sometimes we reuse the same buffer for keyboard input, which means the
        // buffer can have something like this:
        //                   53 54 55 00 30 31 32
        //      this input ends here ->| |<- leftover from the last input
        // we don't just do i=i+1 because this can read into bad input.
        if (!str[i]) { break; }
        i_ = i = i+1;
    }
    *CWD_END = 0;
}

char cwd_get_drive_n() {
    return (read_nibble_ch(CWD_FAR[1])<<4)|read_nibble_ch(CWD_FAR[2]);
}

void cwd_goto_parent() {
    unsigned short len = *CWD_LEN_FAR;
    unsigned short i = len-2;
    while (i > 0 && CWD_FAR[i] != '/') {
        i--;
    }
    if (i > 0) {
        CWD_FAR[i+1] = '\0';
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
