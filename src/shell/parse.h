#ifndef __MINI16_SHELL_PARSE
#define __MINI16_SHELL_PARSE

char* get_token(char* str, char* target);
size_t get_token_i(char* str, char* target);
char read_nibble_ch(char x);
unsigned short int read_word(const char* str);
unsigned char read_byte(const char* str);

#endif
