#ifndef __MINI16_FAT12_FAT12
#define __MINI16_FAT12_FAT12

typedef _Packed struct {
    char jmp[3];
    char oem_name[8];
    unsigned short byte_per_sector;
    char sector_per_cluster;
    unsigned short n_reserved_sector;
    char n_fat;
    unsigned short n_root_entry;
    unsigned short n_sector;
    char media_descriptor;
    unsigned short sector_per_fat;
} FATDescriptor;

typedef _Packed struct {
    char name[8];
    char ext_name[3];
    char attr;
    char f1_0x0c;
    char f2_0x0d;
    unsigned short create_time;
    // create time: 15-11 hours; 10-5 minutes; 4-0 seconds/2
    unsigned short create_date;
    // create date: 15-9 year (0 = 1980)
    //              8-5 month
    //              4-0 day
    unsigned short f5_0x12;
    unsigned short f6_0x14;  // used for unix-like file permission in DR DOS 3.31+
    unsigned short last_modified_time;
    unsigned short last_modified_date;
    unsigned short start_of_file;
    char file_size[4];
} FATDirectoryEntry;


#endif
