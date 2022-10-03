#ifndef __MINI16_SHELL_CWD
#define __MINI16_SHELL_CWD

extern void cwd_goto(const char* str);
extern char cwd_get_drive_n();
extern void cwd_goto_parent();
extern void cwd_set_drive_n(char drive_n);

#endif
