#ifndef __MINI16_FS_FS
#define __MINI16_FS_FS

// maintain a table of these somewhere in the memory. assume the base is BASE_FAR,
// whenever user program receives an integer for file handler, the corresponding
// FSHandler address would be (BASE_FAR + handler_int * sizeof(FSHandler)).
// we only support FAT12 currently.
typedef struct {
    char fstype;
    char drive_n;
    unsigned short cluster_id;
    char far* file_buffer;
    
} FSHandler;

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
