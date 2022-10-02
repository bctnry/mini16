#include "../term/term.h"
#include "../term/cursor.h"
#include "../kb/kb.h"
#include "../cstdlib/string.h"
#include "../util/debug.h"
#include "../util/power.h"
#include "../util/formatstr.h"
#include "../disk/disk.h"

#include "parse.h"

#include "../vesa/vesa.h"
#include "../fs/fs.h"
#include "../fat12/fat12.h"

#define M_MIN(X, Y) (((X)<(Y))?(X):(Y))

const char* PROMPT = "& ";
const char* UNKNOWN_COMMAND = "? Unknown command ";
char cmdbuf[256];
char tokenbuf[256];
char strbuf[256];
const char* AT_FORMAT_ERROR_HEAD = "? Format: ";
const char* AT_READ_MEM_FORMAT = "@read_mem [n] [seg] [off]";
const char* AT_WRITE_MEM_FORMAT = "@write_mem [seg] [off]";
const char* AT_CHKDSK_FORMAT = "@disk_info [drive_n]";
const char* AT_READ_DISK_FORMAT = "@read_disk [n] [seg] [off] [drive_n] [cyl] [head] [sec]";
const char* AT_WRITE_DISK_FORMAT = "@write_disk [n] [seg] [off] [drive_n] [cyl] [head] [sec]";
const char* AT_LBA_FORMAT = "@lba_read/write [drive_n] [n] [seg] [off] [lba_low32]";
const char* AT_MOUNT_FORMAT = "@mount [drive_n] [fs_type]";
const char* WRONG_DATA_FORMAT = "? Wrong data format";
const char* DISK_OPERATION_ERROR = "? Disk operation error";

void _disp_error(const char* errormsg) {
    term_echo_str(errormsg);
    term_echo_newline();
}

void _disp_format_error(const char* errormsg) {
    term_echo_str(AT_FORMAT_ERROR_HEAD);
    term_echo_str(errormsg);
    term_echo_newline();
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
    base = (char far*)((((unsigned long)seg)<<16)+off);
    ix = 0;
    do {
        // TODO: fix this.
        disp_addr(base + ix);
        term_echo_str("   ");

        for (i = 0; i < 16; i++) {
            if (ix >= n) { break; }
            disp_byte(((char far*)base)[ix]);
            term_echo(' ');
            ix++;
        }
        term_echo_newline();
    } while (ix < n);
    term_echo_newline();
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
    base = (char far*)((((unsigned long)seg)<<16)+off);
    writemem_shell(base);
    term_echo_newline();
}

void at_list_disk() {
    DriveParameter dparam;
    unsigned char drive_n;
    drive_n = 0;
    while (!get_drive_param(drive_n, &dparam)) {
        sformat(strbuf, "%B: %B %W %B\n",
            drive_n,
            dparam.max_head,
            dparam.max_cylinder,
            dparam.max_sector
        );
        term_echo_str(strbuf);
        drive_n ++;
    }
    drive_n = 0x80;
    while (!get_drive_param(drive_n, &dparam)) {
        sformat(strbuf, "%B: %B %W %B\n",
            drive_n,
            dparam.max_head,
            dparam.max_cylinder,
            dparam.max_sector
        );
        term_echo_str(strbuf);
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
        sformat(strbuf, "%s: %B\n", DISK_OPERATION_ERROR, char_arg0);
        term_echo_str(strbuf);
    } else {
        sformat(strbuf, "Max Sector: %B\nMax Cylinder: %W\nMax Head: %B\n",
            dparam.max_sector,
            dparam.max_cylinder,
            dparam.max_head
        );
        term_echo_str(strbuf);
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
    if (!read_word_token(&subj, &cylinder)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &head)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &sector)) {
        _disp_format_error(AT_READ_DISK_FORMAT); return;
    }
    base = (char far*)((((unsigned long)seg)<<16)+off);
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
    if (!read_word_token(&subj, &cylinder)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &head)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    if (!read_byte_token(&subj, &sector)) {
        _disp_format_error(AT_WRITE_DISK_FORMAT); return;
    }
    base = (char far*)((((unsigned long)seg)<<16)+off);
    write_sector(base, char_arg0, drive_n, head, cylinder, sector);
    if (get_diskop_status(drive_n)) {
        term_echo_str(DISK_OPERATION_ERROR);
        term_echo_str(": ");
        disp_byte(char_arg0);
        term_echo(0x0d); term_echo(0x0a);
    }
}

// NOTE: a few issues about APM here:
// according to this source:
//     https://wiki.osdev.org/Shutdown
// turn off all devices with BX=01h is only supported with APM 1.1+
// so it cannot support devices with only APM 1.0, which I have zero
// idea what kind of legacy devices that will be.
// qemu shuts down even when at_apm_load is not called beforehand,
// so maybe I did something wrong in the APM driver or that's just
// the way qemu is.
// it was also said that "many new machines does not support APM
// any more". while I'm not anticipating any one will run this on
// actual hardware, I should switch to ACPI shutdown in the future.

void at_apm_load() {
    unsigned short res = power_apm_chk();
    char res2;
    if (!res) {
        term_echo_str("No APM support.\r\n");
        return;
    }
    term_echo_str("APM installed ");
    disp_word(res);
    term_echo_newline();
    res2 = power_apm_connect();
    if (!res2) {
        term_echo_str("Cannot connect to APM interface.\r\n");
        return;
    }
    res2 = power_apm_setver(res);
    if (res2) {
        term_echo_str("Failed to set ver: ");
        disp_byte(res2);
        term_echo_newline();
        power_apm_disconnect();
        return;
    }
    res2 = power_apm_enable_all();
    if (res2) {
        term_echo_str("Failed to enable power management: ");
        disp_byte(res2);
        term_echo_newline();
        power_apm_disconnect();
    }
    term_echo_str("APM interface connected & enabled.\r\n");
}

void at_apm_shutdown() {
    char res = power_apm_set_state(3);
    if (res) {
        term_echo_str("Failed to shutdown: ");
        disp_byte(res);
        term_echo_newline();
    }
}

void at_vesa() {
    VBEInfoStructure s;
    unsigned char i = 0;
    vesa_check_vbe(&s);
    term_echo(s.signature[0]);
    term_echo(s.signature[1]);
    term_echo(s.signature[2]);
    term_echo(s.signature[3]);
    term_echo(' ');
    disp_word(s.version);
    term_echo_newline();
    while (s.video_modes[i] != 0xffff) {
        disp_word(s.video_modes[i]);
        term_echo(' ');
        i++;
    }
    term_echo_newline();
}

void at_lba(unsigned char action, char* x) {
    LBA_Packet pkt;
    char* subj = get_token(x, tokenbuf);
    unsigned char drive;
    char res;
    pkt.zero = 0;
    if (!read_byte_token(&subj, &drive)) {
        _disp_format_error(AT_LBA_FORMAT); return;
    }
    if (!read_word_token(&subj, &pkt.n_sector)) {
        _disp_format_error(AT_LBA_FORMAT); return;
    }
    if (!read_word_token(&subj, &pkt.buffer_segment)) {
        _disp_format_error(AT_LBA_FORMAT); return;
    }
    if (!read_word_token(&subj, &pkt.buffer_offset)) {
        _disp_format_error(AT_LBA_FORMAT); return;
    }
    if (!read_word_token(&subj, (unsigned short*)&pkt.address_low32)) {
        _disp_format_error(AT_LBA_FORMAT); return;
    }
    res = lba_action(drive, action, &pkt);
    if (!res) {
        _disp_error("Error while LBA");
    }
}

void at_mount(char* x) {
    char drive_n, type;
    char* subj = get_token(x, tokenbuf);
    if (!read_byte_token(&subj, &drive_n)) {
        _disp_format_error(AT_MOUNT_FORMAT); return;
    }
    if (!read_byte_token(&subj, &type)) {
        _disp_format_error(AT_MOUNT_FORMAT); return;
    }
    fs_mount(drive_n, type);
}

void at_fat12_info(char* x) {
    char drive_n;
    char* subj = get_token(x, tokenbuf);
    unsigned short maximum_root_len;
    unsigned char i = 0;
    size_t vx = 0;
    unsigned char ix = 0;
    char far* z = ((char far*)(0x0000d000));
    if (!read_byte_token(&subj, &drive_n)) {
        return;
    }

    read_sector(
        z,
        1,
        drive_n,
        0, 0, 1
    );

    maximum_root_len = ((FATDescriptor far*)z)->n_root_entry;
    sformat(strbuf, "Max root entries: %W\n", maximum_root_len);
    term_echo_str(strbuf);
    term_echo_str("OEM Name: ");
    for (vx = 0; vx < 8; vx++) {
        term_echo(((FATDescriptor far*)z)->oem_name[vx]);
    }
    term_echo_newline();
    sformat(strbuf, "Byte per sector: %W\nSector per cluster: %W\nReserved sector: %W\n",
        ((FATDescriptor far*)z)->byte_per_sector,
        ((FATDescriptor far*)z)->sector_per_cluster,
        ((FATDescriptor far*)z)->n_reserved_sector
    );
    term_echo_str(strbuf);
    // TODO: somehow the last field displays wrong. fix this.
    sformat(strbuf, "No. FAT: %B\nNo. root entry: %W\nDescriptor: %B\nSector per FAT: %W\n",
        ((FATDescriptor far*)z)->n_fat,
        ((FATDescriptor far*)z)->n_root_entry,
        ((FATDescriptor far*)z)->n_sector,
        ((FATDescriptor far*)z)->media_descriptor,
        ((FATDescriptor far*)z)->sector_per_fat
    );
}

// char far* CWD = ((char far*)0x00000500);
char* CWD = ((char*)0x0500);

void at_chd(char* x) {
    char drive_n;
    char* subj = get_token(x, tokenbuf);
    char z;
    if (!read_byte_token(&subj, &drive_n)) {
        term_echo_str("? No drive");
        return;
    }
    z = drive_n >> 4;
    CWD[1] = z >= 0x0a? z + 'A' - 0x0a : z + 0x30;
    drive_n %= 0xf;
    CWD[2] = drive_n >= 0x0a? drive_n + 'A' - 0x0a : drive_n + 0x30;
    term_echo_str(CWD);
    term_echo_newline();
}
void get_cluster(char* x);
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
    } else if (strcmp(tokenbuf, "@apm_load") == 0) {
        at_apm_load();
    } else if (strcmp(tokenbuf, "@apm_shutdown") == 0) {
        at_apm_shutdown();
    } else if (strcmp(tokenbuf, "@vesa_chk") == 0) {
        at_vesa();
    } else if (strcmp(tokenbuf, "@lba_read") == 0) {
        at_lba(DISK_LBA_READ, x);
    } else if (strcmp(tokenbuf, "@lba_write") == 0) {
        at_lba(DISK_LBA_WRITE, x);
    } else if (strcmp(tokenbuf, "@mount") == 0) {
        at_mount(x);
    } else if (strcmp(tokenbuf, "@fat12_info") == 0) {
        at_fat12_info(x);
    } else if (strcmp(tokenbuf, "@chd") == 0) {
        at_chd(x);
    } else {
        term_echo_str(UNKNOWN_COMMAND);
        term_echo_str(x);
        term_echo_newline();
    }
    return;
}

char far* DISKBUFF = (char far*)0x0000d000;
char far* FATBUFF = (char far*)0x0000d400;


// NOTE: this function always read from FAT1.
// cluster_id shouldn't be bigger than:
//     (fat_desc.sector_per_fat * fat_desc.byte_per_sector) / 1.5
// for any cluster_id (start from 0)
//     if even, lower byte at: cluster_id/2*3
//              higher nibble at: (cluster_id)/2*3+1 lower nibble
//     if odd, lower nibble at: cluster_id/2*3+1 higher nibble
//             higher byte at: cluster_id/2*3+2
// this will be the offset.
// from the offset we calculate which sector of the FAT should be loaded.
//     nth = offset / fat_desc.byte_per_sector
/*
unsigned short fat12_get_cluster(DriveParameter* dp, unsigned short cluster_id) {
    FATDescriptor far* fat_desc;
    unsigned short n;
    unsigned short offset;
    unsigned short nth;
    unsigned short res;
    char head, sector;
    char drive_n;
    unsigned short cylinder;
    drive_n = (read_nibble_ch(CWD[1])<<4)|read_nibble_ch(CWD[2]);
    fat_desc = (FATDescriptor far*)DISKBUFF;
    n = fat_desc->n_reserved_sector;
    offset = cluster_id/2*3+(cluster_id&1?1:0);
    nth = offset / fat_desc->byte_per_sector;
    lbs_to_chs(dp, n, &head, &sector, &cylinder);
    read_sector(
        FATBUFF,
        1,
        drive_n,
        head,
        cylinder,
        sector
    );
    offset %= fat_desc->byte_per_sector;
    res = (
        cluster_id&1? FATBUFF[offset]>>4|(FATBUFF[offset+1]<<4)
        : FATBUFF[offset]|(FATBUFF[offset+1]&0xf)
    );
    return res;
}

void get_cluster(char* x) {
    DriveParameter dp;
    char* subj = get_token(x, tokenbuf);
    unsigned short n;
    unsigned short res;
    char drive_n;
    
    drive_n = (read_nibble_ch(CWD[1])<<4)|read_nibble_ch(CWD[2]);
    get_drive_param(drive_n, &dp);
    read_word_token(&subj, &n);
    res = fat12_get_cluster(&dp, n);
    disp_word(res);
    term_echo_newline();
}
*/
void ls() {
    // drive: CMD+1, CMD+2
    // path start: CMD+3
    // 1.  check drive.
    // 2.  check DMRT.
    // 3.  unsupported 
    char drive_n;
    char sector_per_cluster;
    char subj = CWD[4];
    char head, sec;
    size_t i = 0, i2 = 0, page_i = 0, whole_i = 0;
    unsigned short cyl;
    // NOTE: this will only support drives less than ~32MBytes.
    unsigned short temp;
    char bound;
    DriveParameter dp;
    FATDirectoryEntry far* dir_entry_pointer;

    drive_n = (read_nibble_ch(CWD[1])<<4)|read_nibble_ch(CWD[2]);
    get_drive_param(drive_n, &dp);
    switch (fs_gettype(drive_n)) {
        case 0: {
            term_echo_str("? Drive not mounted\n"); return;
        }
        case 1: {
            goto fat12;
        }
        default: {
            sformat(strbuf, "? Mount type not supported: %B\n", fs_gettype(drive_n));
            term_echo_str(strbuf);
            return;
        }
    }

    fat12:
    // read first sector into diskbuff.
    read_sector(DISKBUFF, 1, drive_n, 0, 0, 1);
    sector_per_cluster = ((FATDescriptor*)DISKBUFF)->sector_per_cluster;
    // find & read dir.
    temp = (
        ((FATDescriptor*)DISKBUFF)->n_reserved_sector
        + ((FATDescriptor*)DISKBUFF)->sector_per_fat
            * ((FATDescriptor*)DISKBUFF)->n_fat
    );
    disp_word(temp); term_echo_newline();
    // NOTE: first bound is the root dir bound.
    bound = ((FATDescriptor*)DISKBUFF)->byte_per_sector / 32;

    // chs -> nsec = c * (dp.sec*dp.h) + h * (dp.sec) + s
    // nsec -> chs = (
    //      nsec / dp.sec / dp.h
    //      nsec / dp.sec % dp.h
    //      nsec % dp.sec
    // )
    lba_to_chs(&dp, temp, &head, &sec, &cyl);
    dir_entry_pointer = ((FATDirectoryEntry far*)(DISKBUFF + ((FATDescriptor*)DISKBUFF)->byte_per_sector));
    read_sector((char far*)dir_entry_pointer,
        1,
        drive_n,
        head,
        cyl,
        sec
    );
    // while whole_i < bound, the whole dir is not checked.
    // a dir can span multiple page.
    // while no entry in current page matches:
    //     1. read next page.
    //     2. rinse and repeat
    // while a entry is match to current request:
    //     1. get starting_cluster.
    //     1. read the cluster.
    //     2. 
    /*
    while (whole_i < bound) {
        while (page_i < page_len && dir_entry_pointer[page_i].name[0]) {
            // check each page.
            i = 0; i2 = 0;
            while (
                subj[i]
                && subj[i] != '/'
                && i < 8
                && dir_entry_pointer[page_i].name[i] != ' '
                && subj[i] == dir_entry_pointer[page_i].name[i]
            ) {
                i++;
            }
            // NOTE: the following are the condition for continuing to the match for ext_name.
            // if we failed at this point we should just go to the next item.
            if (!(!subj[i] || subj[i] == '/' || i == 8 || dir_entry_pointer[page_i].name[i] == ' ')) {
                page_i++;
                continue;
            }
            while (
                subj[i]
                && subj[i] != '/'
                && i2 < 3
                && dir_entry_pointer->ext_name[i2] != ' '
                && subj[i] == dir_entry_pointer->name[i2]
            ) {
                i++;
                i2++;
            }
            if (!(!subj[i] || subj[i] == '/' || i == 8 || dir_entry_pointer[page_i].name[i] == ' ')) {
                page_i++;
                continue;
            }
            subj = &subj[i+1];
            page_i ++;
        }

    }
    */
}

void _shell(void) {
    size_t i = 0;
    char* x = cmdbuf;
    cursor_set_pos(0, 24);
    for (;;) {
        term_echo_str(PROMPT);
        kb_readline(cmdbuf, 256, 0, 0);
        while (cmdbuf[i] && (cmdbuf[i] == ' ' || cmdbuf[i] == '\t')) { i++; }
        if (!cmdbuf[0]) { continue; }
        x = &cmdbuf[i];
        if (x[0] == '@') {
            at_shell(x);
        } else if (strcmp(x, "ver") == 0) {
            term_echo_str("Mini16 2022.10.2\n");
        } else if (strcmp(x, "exit") == 0) {
            at_apm_shutdown();
            break;
        } else if (strcmp(x, "ls") == 0) {
            ls();
        } else if (strcmp(x, "cwd") == 0) {
            term_echo_str(CWD);
            term_echo_newline();
        } else {
            term_echo_str(UNKNOWN_COMMAND);
            term_echo_str(x);
            term_echo_newline();
        }
    }
}

void init_cwd() {
    strcpy(CWD, "/00/");
}

void shell(void) {
    init_cwd();
    _shell();
}
