#include "../term/vga.h"
#include "../term/term.h"
#include "../term/cursor.h"
#include "../kb/kb.h"
#include "../cstdlib/string.h"
#include "../util/debug.h"
#include "../disk/disk.h"
#include "parse.h"

#define M_MIN(X, Y) (((X)<(Y))?(X):(Y))

const char* PROMPT = "& ";
const char* UNKNOWN_COMMAND = "? Unknown command ";
char cmdbuf[256];
char tokenbuf[256];
const char* AT_FORMAT_ERROR_HEAD = "? Format: ";
const char* AT_READ_MEM_FORMAT = "@read_mem [n] [seg] [off]";
const char* AT_WRITE_MEM_FORMAT = "@write_mem [seg] [off]";
const char* AT_CHKDSK_FORMAT = "@disk_info [drive_n]";
const char* AT_READ_DISK_FORMAT = "@read_disk [n] [seg] [off] [drive_n] [head] [cyl] [sec]";
const char* AT_WRITE_DISK_FORMAT = "@write_disk [n] [seg] [off] [drive_n] [head] [cyl] [sec]";
const char* WRONG_DATA_FORMAT = "? Wrong data format";
const char* DISK_OPERATION_ERROR = "? Disk operation error";

void _disp_error(const char* errormsg) {
    vga_write_str(errormsg, 0, 24);
    vga_flush_up(0, 25, 1, 1);
}

void _disp_format_error(const char* errormsg) {
    vga_write_str(AT_FORMAT_ERROR_HEAD, 0, 24);
    vga_write_str(errormsg, 10, 24);
    vga_flush_up(0, 25, 1, 1);
}

char read_word_token(char** x, unsigned short* n) {
    *x = get_token(*x, tokenbuf);
    if (!tokenbuf[0]) { return 0; }
    *n = read_word(tokenbuf);
    return 1;
}

char read_byte_token(char** x, unsigned char* n) {
    *x = get_token(*x, tokenbuf);
    if (!tokenbuf[0]) { return 0; }
    *n = read_byte(tokenbuf);
    return 1;
}

char _writemem_getbyte(char* res) {
    if (!isxdigit(cmdbuf[0]) || !isxdigit(cmdbuf[1])) { return 0; }
    *res = (read_nibble_ch(cmdbuf[0])<<4)|read_nibble_ch(cmdbuf[1]);
    return 1;
}

void writemem_shell(char far* base) {
    char res = 0;
    for (;;) {
        disp_addr(base);
        term_echo_str("   ");
        disp_byte(base[0]);
        term_echo_str("  -> ");
        kb_readline(cmdbuf, 256, 0, 0);
        if (!cmdbuf[0]) { break; }
        if (!_writemem_getbyte(&res)) {
            _disp_error(WRONG_DATA_FORMAT); continue;
        }
        *base = res;
        base++;
    }

}

void at_read_mem(char* x) {
    char* subj = get_token(x, tokenbuf);
    unsigned short n, seg, off, i, ix;
    char far* base;
    if (!read_word_token(&subj, &n)) {
        _disp_format_error(AT_READ_MEM_FORMAT); return;
    }
    if (n == 0) { return; }
    if (!read_word_token(&subj, &seg)) {
        _disp_format_error(AT_READ_MEM_FORMAT); return;
    }
    if (!read_word_token(&subj, &off)) {
        _disp_format_error(AT_READ_MEM_FORMAT); return;
    }
    base = (((unsigned long)seg)<<16)+off;
    ix = 0;
    do {
        disp_word(seg); term_echo(':'); disp_word(off + (ix & 0xf0)); term_echo_str("   ");
        for (i = 0; i < 16; i++) {
            if (ix >= n) { break; }
            disp_byte(base[off + ix]);
            term_echo(' ');
            ix++;
        }
        term_echo_newline();
    } while (ix < n);
    vga_flush_up(0, 25, 1, 1);
}

void at_write_mem(char* x) {
    char* subj = get_token(x, tokenbuf);
    unsigned short n, seg, off;
    char far* base;
    if (!read_word_token(&subj, &seg)) {
        _disp_format_error(AT_WRITE_MEM_FORMAT); return;
    }
    if (!read_word_token(&subj, &off)) {
        _disp_format_error(AT_WRITE_MEM_FORMAT); return;
    }
    base = (((unsigned long)seg)<<16)+off;
    writemem_shell(base);
    vga_flush_up(0, 25, 1, 1);
}

void at_list_disk() {
    DriveParameter dparam;
    unsigned char drive_n;
    drive_n = 0;
    while (!get_drive_param(drive_n, &dparam)) {
        disp_byte(drive_n); term_echo_str(": ");
        disp_byte(dparam.max_head); term_echo(' ');
        disp_word(dparam.max_cylinder); term_echo(' ');
        disp_byte(dparam.max_sector); term_echo_newline();
        drive_n ++;
    }
    drive_n = 0x80;
    while (!get_drive_param(drive_n, &dparam)) {
        disp_byte(drive_n); term_echo_str(": ");
        disp_byte(dparam.max_head); term_echo(' ');
        disp_word(dparam.max_cylinder); term_echo(' ');
        disp_byte(dparam.max_sector); term_echo_newline();
        drive_n ++;
    }
}

void at_disk_info(char* x) {
    char* subj = get_token(x, tokenbuf);
    unsigned char drive_n, char_arg0;
    DriveParameter dparam;
    if (!read_byte_token(&subj, &drive_n)) {
        _disp_format_error(AT_CHKDSK_FORMAT); return;
    }
    char_arg0 = get_drive_param(drive_n, &dparam);
    if (char_arg0) {
        term_echo_str(DISK_OPERATION_ERROR);
        term_echo_str(": ");
        disp_byte(char_arg0);
        term_echo(0x0d); term_echo(0x0a);
    } else {
        term_echo_str("Max Sector: ");
        disp_byte(dparam.max_sector);
        term_echo_newline();
        term_echo_str("Max Cylinder: ");
        disp_word(dparam.max_cylinder);
        term_echo_newline();
        term_echo_str("Max Head: ");
        disp_byte(dparam.max_head);
        term_echo_newline();
    }
}

void at_read_disk(char* x) {
    char* subj = get_token(x, tokenbuf);
    unsigned char drive_n, char_arg0, head, sector;
    unsigned short seg, off, cylinder;
    char far* base;
    if (!read_byte_token(&subj, &char_arg0)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_word_token(&subj, &seg)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_word_token(&subj, &off)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &drive_n)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &head)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_word_token(&subj, &cylinder)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &sector)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    base = (((unsigned long)seg)<<16)+off;
    read_sector(base, char_arg0, drive_n, head, cylinder, sector);
    if (get_diskop_status(drive_n)) {
        term_echo_str(DISK_OPERATION_ERROR);
        term_echo_str(": ");
        disp_byte(char_arg0);
        term_echo(0x0d); term_echo(0x0a);
    }
}

void at_write_disk(char* x) {
    char* subj = get_token(x, tokenbuf);
    unsigned char drive_n, char_arg0, head, sector;
    unsigned short seg, off, cylinder;
    char far* base;
    if (!read_byte_token(&subj, &char_arg0)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_word_token(&subj, &seg)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_word_token(&subj, &off)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &drive_n)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &head)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_word_token(&subj, &cylinder)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &sector)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    base = (((unsigned long)seg)<<16)+off;
    write_sector(base, char_arg0, drive_n, head, cylinder, sector);
    if (get_diskop_status(drive_n)) {
        term_echo_str(DISK_OPERATION_ERROR);
        term_echo_str(": ");
        disp_byte(char_arg0);
        term_echo(0x0d); term_echo(0x0a);
    }
}

// `x` already trimmed left.
void at_shell(char* x) {
    char* subj = get_token(x, tokenbuf);
    if (strcmp(tokenbuf, "@read_mem") == 0) {
        at_read_mem(x);
    } else if (strcmp(tokenbuf, "@write_mem") == 0) {
        at_write_mem(x);
    } else if (strcmp(tokenbuf, "@disk_info") == 0) {
        at_disk_info(x);
    } else if (strcmp(tokenbuf, "@read_disk") == 0) {
        at_read_disk(x);
    } else if (strcmp(tokenbuf, "@write_disk") == 0) {
        at_write_disk(x);
    } else if (strcmp(tokenbuf, "@list_disk") == 0) {
        at_list_disk();
    } else {
        vga_write_str(UNKNOWN_COMMAND, 0, 24);
        vga_write_str(x, strlen(UNKNOWN_COMMAND), 24);
        vga_flush_up(0, 25, 1, 1);
    }
    return;
}


void shell(void) {
    size_t i = 0;
    char* x = cmdbuf;
    vga_write_str_c("Mini16 2022.5.27", COLOR_BG_BLACK|COLOR_FG_WHITE|COLOR_FG_BRIGHT, 0, 23);
    for (;;) {
        vga_set_color(COLOR_BG_BLACK|COLOR_FG_WHITE|COLOR_FG_BRIGHT, 0, 24, 2, 25);
        vga_write_str(PROMPT, 0, 24);
        cursor_set_pos(2, 24);
        kb_readline(cmdbuf, 256, 0, 0);
        while (cmdbuf[i] && (cmdbuf[i] == ' ' || cmdbuf[i] == '\t')) { i++; }
        x = &cmdbuf[i];
        if (x[0] == '@') {
            at_shell(x);
        } else if (strcmp(x, "exit") == 0) {
            break;
        } else if (strcmp(x, "clear") == 0) {
            vga_flush_up(0, 25, 0, 1);
        } else {
            // vga_flush_up(0, 25, 1, 1);
            vga_write_str(UNKNOWN_COMMAND, 0, 24);
            vga_write_str(x, strlen(UNKNOWN_COMMAND), 24);
            vga_flush_up(0, 25, 1, 1);
        }
    }
}
