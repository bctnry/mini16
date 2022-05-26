/* Mini16 VGA text mode. 16-color 80x25.  */
#include "vga.h"
#include "cursor.h"

#define VIDEO_BUFFER ((char far *)0xb8000000)

#define size_t unsigned short


void vga_write_ch(char ch, char x, char y) {
    VIDEO_BUFFER[y * 160 + x * 2] = ch;
}
void vga_write_ch_c(char ch, char color, char x, char y) {
    VIDEO_BUFFER[y * 160 + x * 2 + 1] = color;
    VIDEO_BUFFER[y * 160 + x * 2] = ch;
}

void vga_write_str(const char* str, char x, char y) {
    size_t i = 0;
    size_t base = y * 160 + x * 2;
    while (str[i]) {
        VIDEO_BUFFER[base] = str[i];
        i++;
        base += 2;
    }
}
void vga_write_str_c(const char* str, char color, char x, char y) {
    size_t i = 0;
    size_t base = y * 160 + x * 2;
    while (str[i]) {
        VIDEO_BUFFER[base] = str[i];
        VIDEO_BUFFER[base + 1] = color;
        i++;
        base += 2;
    }
}

void vga_set_color(char color, char x1, char y1, char x2, char y2) {
    size_t i, j;
    for (j = y1; j < y2; j++) {
        for (i = x1; i < x2; i++) {
            VIDEO_BUFFER[j * 160 + i * 2 + 1] = color;
        }
    }
}

char vga_read_ch(char x, char y) {
    return VIDEO_BUFFER[y * 160 + x * 2];
}

char vga_read_color(char x, char y) {
    return VIDEO_BUFFER[y * 160 + x * 2 + 1];
}

void vga_flush_up(char start, char end, char up_line_cnt, char move_color) {
    /* TODO: optimize this.  */
    size_t start_base, target_start_base, workload, i, current_line;
    signed short target_start;
    start_base = (unsigned short)start * 160;

    for (current_line = start; current_line < end; current_line++) {
        start_base = current_line * 160;
        target_start = (signed short)current_line - up_line_cnt;
        if (target_start < 0) { target_start = 0; }
        target_start_base = (unsigned short)target_start * 160;
        for (i = 0; i < 160; i+=2) {
            VIDEO_BUFFER[target_start_base + i] = VIDEO_BUFFER[start_base + i];
            VIDEO_BUFFER[start_base + i] = 0;
            if (move_color) {
                VIDEO_BUFFER[target_start_base + i + 1] = VIDEO_BUFFER[start_base + i + 1];
                VIDEO_BUFFER[start_base + i + 1] = COLOR_BG_BLACK|COLOR_FG_WHITE;
            }
        }
    }
}

void vga_write_ch_at_cursor(char ch) {
    unsigned short pos = cursor_get_pos();
    vga_write_ch(ch, (char)pos&0xff, (char)(pos>>8));
    return;
}

void vga_write_ch_at_cursor_c(char ch, char color) {
    unsigned short pos = cursor_get_pos();
    vga_write_ch_c(ch, color, (char)(pos&0xff), (char)(pos>>8));
    return;
}

