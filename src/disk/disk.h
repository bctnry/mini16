#ifndef __MINI16_DISK_DISK
#define __MINI16_DISK_DISK

extern char read_sector(
    char far* buf,
    unsigned char n,
    unsigned char drive,
    unsigned char head,
    unsigned short cylinder,
    unsigned char sector
);

extern void write_sector(
    char far* buf,
    unsigned char n,
    unsigned char drive,
    unsigned char head,
    unsigned short cylinder,
    unsigned char sector
);

typedef struct {
    unsigned char max_head;
    unsigned char max_sector;
    unsigned short max_cylinder;
} DriveParameter;

extern char get_diskop_status(char drive);
extern char get_drive_param(char drive, DriveParameter* param);

#endif
