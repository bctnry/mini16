#ifndef __MINI16_FS_FS
#define __MINI16_FS_FS

// extern fs_chdir(const char* target_path);
// extern fs_open_file(const char* path);
// FAILURE_ON_ZERO
extern char fs_list_dir(char drive_n, const char* ls);

#define FS_TYPE_UNSPECIFIED 0x0
#define FS_TYPE_FAT12 0x1
#define FS_TYPE_FAT16 0x2
extern void fs_mount(char drive_n, char type);
extern char fs_gettype(char drive_n);

extern char far* fs_getcwd();

#endif
