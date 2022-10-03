#include "../cstdlib/string.h"
#include "../term/vga.h"

char* get_token(char* str, char* target) {
    size_t i = 0, j = 0;
    while (str[i] && (str[i] == ' ' || str[i] == '\t')) { i++; }
    while (str[i] && str[i] != ' ' && str[i] != '\t') {
        target[j] = str[i];
        i++;
        j++;
    }
    target[j] = 0;
    return &str[i];
}

inline char read_nibble_ch(char x) {
    char res = x >= 'a'? (x - 'a' + 0x0a) : x >= 'A'? (x - 'A' + 0x0a) : x - '0';
    return res;
}

unsigned short int read_word(const char* str) {
    unsigned short int res = 0;
    size_t i = 0;
    if (strncmp(str, "0x", 2) == 0) {
        // hex.
        i = 2; while (str[i]) {
            res <<= 4;
            res += (unsigned short int)read_nibble_ch(str[i]);
            i++;
        }
    } else {
        while (str[i]) {
            res *= 10;
            res += str[i] - '0';
            i++;
        }
    }
    return res;
}

unsigned char read_byte(const char* str) {
    return (unsigned char)read_word(str);
}
