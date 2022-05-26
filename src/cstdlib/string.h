#ifndef __MINI16_CSTDLIB_STRING
#define __MINI16_CSTDLIB_STRING

#ifndef size_t
#define size_t unsigned short
#endif

int strcmp(const char* lhs, const char* rhs);
int strncmp(const char* lhs, const char* rhs, size_t count);
char* strchr(const char* str, char ch);
size_t strlen(const char* str);

char isdigit(char x);
char isxdigit(char x);

#endif
