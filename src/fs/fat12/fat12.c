#include "../const.h"
#include "fat12.h"
#include "../disk/disk.h"
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

