#ifndef __MINI16_UTIL_FORMATSTR
#define __MINI16_UTIL_FORMATSTR

#include<stdarg.h>

void sformat(char* target, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    while ((*fmt != '\0')) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'B': {
                    unsigned char ch = va_arg(args, unsigned char);
                    unsigned char ch2 = ch & 0x0f;
                    ch >>= 4;
                    *target = ch >= 0x0a? ch + 'A' - 0x0a : ch + 0x30; target++;
                    *target = ch2 >= 0x0a? ch2 + 'A' - 0x0a : ch2 + 0x30; target++;
                    fmt++;
                    break;
                }
                case 'W': {
                    unsigned short word = va_arg(args, unsigned short);
                    unsigned char ch = word>>12;
                    *target = ch >= 0x0a? ch + 'A' - 0x0a : ch + 0x30; target++;
                    ch = (word&0x0fff)>>8;
                    *target = ch >= 0x0a? ch + 'A' - 0x0a : ch + 0x30; target++;
                    ch = (word&0x00ff)>>4;
                    *target = ch >= 0x0a? ch + 'A' - 0x0a : ch + 0x30; target++;
                    ch = (word&0x000f);
                    *target = ch >= 0x0a? ch + 'A' - 0x0a : ch + 0x30; target++;
                    fmt++;
                    break;
                }

                case 's': {
                    char* substr = va_arg(args, char*);
                    while (*substr) {
                        *target = *substr;
                        substr++;
                        target++;
                    }
                    fmt++;
                    break;
                }

                case 'i': {
                    break;
                }

                case '%':
                    *target = '%';
                    target++;
                    fmt++;
                    break;
            }
        } else {
            *target = *fmt;
            target++;
            fmt++;
        }
    }
    *target = 0;
    va_end(args);
}

#endif
