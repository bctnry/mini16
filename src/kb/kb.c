#include "kb.h"
#include "../term/vga.h"
#include "../term/cursor.h"
#include "../term/term.h"

/* Ctrl+Enter inputs linefeed; so this is also used to handle "pseudo-multi-line" input in Mini16. */
signed char kb_readline(char* buffer, unsigned short bufsize, char no_echo, char no_newline) {
    unsigned short key, curpos;
    char chk;
    unsigned short i = 0;
    while (i < bufsize) {
        /* this is to make sure INT8 can actually be received.
           INT8 is reserved for implementing preemptive multitasking.
           `kb_get_key` probably blocks in the INT16h interrupt handler
           where IF is cleared; this will prevent INT8 from getting called.
           I haven't checked on this yet but I'm not willing to take
           the risk. */
        while (!(chk = kb_check_key())) {};
        /* This will probably cause some problem.  */
        key = kb_get_ascii();
        if (!no_echo) { term_echo(key); }
        switch (key) {
            case 0x0d: {
                buffer[i] = 0;
                if (!no_newline) {
                    term_echo(0x0a);
                }
                return 0;
            }
            case 0x0a: {
                buffer[i] = key;
                term_echo(0x0d);
                break;
            }
            case 0x08: {
                if (i > 0) {
                    i--;
                    vga_write_ch_at_cursor(' ');
                } else {
                    curpos = cursor_get_pos();
                    curpos++;
                    cursor_set_pos((char)((curpos&0xff)), (char)((curpos>>8)));
                }
                break;
            }
            default: {
                buffer[i] = key;
                i++;
                break;
            }
        }
    }
    if (i == bufsize) {
        return KB_OVERFLOW;
    }
    return 0;
}
