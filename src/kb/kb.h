/* Mini16 INT 16h wrapper.  */

#ifndef __MINI16_INT_16
#define __MINI16_INT_16

#define KB_INSERT 0x80
#define KB_CAPSLOCK 0x40
#define KB_NUMLOCK 0x20
#define KB_SCRLOCK 0x10
#define KB_ALT_ON 0x08
#define KB_CTRL_ON 0x04
#define KB_LSHIFT_ON 0x02
#define KB_RSHIFT_ON 0x01

extern char kb_get_flag();
extern char kb_get_scan_code();
extern char kb_get_ascii();
/* This directly returns the result of AH=0;INT16h.  */
extern unsigned short kb_get_key();
/* This is a boolean function.  */
extern char kb_check_key();

/* High-level functions.  */
#define KB_OVERFLOW -1
extern signed char kb_readline(char* buffer, unsigned short bufsize, char no_echo, char no_newline);

#endif
