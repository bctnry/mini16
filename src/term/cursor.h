#ifndef __MINI16_TERM_CURSOR
#define __MINI16_TERM_CURSOR

/* 1Byte row at high & 1Byte column at low.   */
extern unsigned short cursor_get_pos();
extern void cursor_set_pos(char x, char y);


#endif
