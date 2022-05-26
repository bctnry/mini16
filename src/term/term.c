#include "term.h"

void term_echo_newline(void) {
    term_echo(0x0d); term_echo(0x0a);
}

void term_echo_str(const char* x) {
    unsigned short i = 0;
    while (x[i]) {
        switch (x[i]) {
            case 0x0d: case 0x0a: {
                term_echo(0x0d); term_echo(0x0a);
                i++;
                break;
            }
            default: {
                term_echo(x[i]);
                i++;
                break;
            }
        }
    }
}

