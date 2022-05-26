#include "string.h"

int strcmp(const char* lhs, const char* rhs) {
    unsigned short i = 0;
    char a, b;
    for (;;) {
        a = lhs[i];
        b = rhs[i];
        if (!(a && b)) { break; }

        if (a < b) { return -1; }
        if (a > b) { return 1; }
        i++;
    }
    return (
        (a==0&&b==0)? 0
        : !a? -1
        : 1
    );
}

int strncmp(const char* lhs, const char* rhs, size_t count) {
    unsigned short i = 0;
    char a, b;
    while (i < count) {
        a = lhs[i];
        b = rhs[i];
        if (!(a && b)) { break; }
        if (a < b) { return -1; }
        if (a > b) { return 1; }
        i++;
    }
    if (!a && b) { return -1; }
    if (a && !b) { return 1; }
    return 0;
}

char* strchr(const char* str, char ch) {
    size_t i = 0;
    while (str[i] && (str[i] != ch)) { i++; }
    if (!str[i]) { return (char*)0; }
    return &str[i];
}

size_t strlen(const char* str) {
    size_t i = 0;
    while (str[i]) { i++; }
    return i;
}

char isdigit(char x) {
    return x >= '0' && x <= '9';
}

char isxdigit(char x) {
    return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F');
}
