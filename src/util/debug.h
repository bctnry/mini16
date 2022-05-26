#ifndef __MINI16_UTIL_DEBUG
#define __MINI16_UTIL_DEBUG

#include "../term/term.h"

void disp_byte(char x) {
    char z = x>>4;
    x = x&0x0f;
    term_echo(z >= 0x0a? z + 'A' - 0x0a : z + 0x30);
    term_echo(x >= 0x0a? x + 'A' - 0x0a : x + 0x30);
}
void disp_word(unsigned short x) {
    disp_byte((char)(x>>8));
    disp_byte((char)(x&0xff));
}

void disp_dword(unsigned long x) {
    disp_word((unsigned short)(x>>16));
    disp_word((unsigned short)(x&0xffff));
}

void disp_addr(void far* x) {
    disp_word((unsigned short)((unsigned long)x>>16));
    term_echo(':');
    disp_word((unsigned short)(((unsigned long)x)&0xffff));
}


#endif
