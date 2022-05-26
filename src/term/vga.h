/* Mini16 VGA text mode. 16-color 80x25.  */

#ifndef __MINI16_TERM_VGA
#define __MINI16_TERM_VGA

#define COLOR_BLINK 0x80
#define COLOR_BG_BRIGHT 0x80
#define COLOR_BG_BLACK 0x00
#define COLOR_BG_BLUE 0x10
#define COLOR_BG_GREEN 0x20
#define COLOR_BG_CYAN 0x30
#define COLOR_BG_RED 0x40
#define COLOR_BG_MAGENTA 0x50
#define COLOR_BG_BROWN 0x60
#define COLOR_BG_WHITE 0x70

#define COLOR_FG_BRIGHT 0x08
#define COLOR_FG_BLACK 0x00
#define COLOR_FG_BLUE 0x01
#define COLOR_FG_GREEN 0x02
#define COLOR_FG_CYAN 0x03
#define COLOR_FG_RED 0x04
#define COLOR_FG_MAGENTA 0x05
#define COLOR_FG_BROWN 0x06
#define COLOR_FG_WHITE 0x07

extern void vga_write_ch(char ch, char x, char y);
extern void vga_write_str(const char* str, char x, char y);
extern void vga_write_ch_c(char ch, char color, char x, char y);
extern void vga_write_str_c(const char* str, char color, char x, char y);
extern void vga_set_color(char color, char x1, char y1, char x2, char y2);
extern char vga_read_ch(char x, char y);
extern char vga_read_color(char x, char y);

/** character-only flush up. the color will stay in position.  */
extern void vga_flush_up(char start, char end, char line_cnt, char move_color);


extern void vga_write_ch_at_cursor(char ch);
extern void vga_write_ch_at_cursor_c(char ch, char color);

#endif
