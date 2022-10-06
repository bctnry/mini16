#ifndef __MINI16_TERM_TERM
#define __MINI16_TERM_TERM


extern void term_echo(char x);
extern void term_echo_str(const char far* x);
extern void term_echo_newline(void);
extern void term_echo_nstr(const char far* x, unsigned short n);

#endif

