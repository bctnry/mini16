#ifndef __MINI16_DISK_DISK
#define __MINI16_DISK_DISK
#include "../util/visual_hint.h"

typedef struct DriveParameter {
    unsigned char max_head;
    unsigned char max_sector;
    unsigned short int max_cylinder;
    unsigned char id;
} DriveParameter;

typedef struct {
    unsigned char packet_size;
    // NOTE: this field MUST BE ZERO when using.
    unsigned char zero;
    unsigned short int n_sector;
    unsigned short int buffer_offset;
    unsigned short int buffer_segment;
    unsigned long int address_low32;
    unsigned long int address_high32;
} LBA_Packet;

extern char read_sector(
    char far* buf,
    unsigned char n,
    unsigned char drive,
    unsigned char head,
    unsigned short int cylinder,
    unsigned char sector
);

extern void write_sector(
    char far* buf,
    unsigned char n,
    unsigned char drive,
    unsigned char head,
    unsigned short int cylinder,
    unsigned char sector
);

extern char check_lba_support();
#define DISK_LBA_READ 0x42
#define DISK_LBA_WRITE 0x43
extern char FAILURE_AT_ZERO lba_action(
    unsigned char drive,
    unsigned char action,
    LBA_Packet* packet
);
extern char lba_write_sector(
    unsigned char far* buf,
    unsigned char n,
    unsigned long int lba_low32,
    unsigned long int lba_high32
);
extern char get_diskop_status(char drive);
extern char get_drive_param(char drive, DriveParameter* param);

void lba_to_chs(DriveParameter far* param,
    unsigned short n,
    char* head,
    char* sector,
    unsigned short* cylinder
);
void chs_to_lba(DriveParameter far* param,
    char head,
    char sector,
    unsigned short cylinder,
    unsigned short* n
);

#endif
