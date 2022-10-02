#ifndef __MINI16_VESA_VESA
#define __MINI16_VESA_VESA

typedef struct {
    unsigned char signature[4];
    unsigned short version;
    unsigned long oem;
    unsigned long capabilities;
    unsigned short far * video_modes;
    unsigned short video_memory;
    unsigned short software_rev;
    unsigned long vendor;
    unsigned long product_name;
    unsigned long product_rev;
    char reserved[222];
    char oem_data[256];
} VBEInfoStructure;


extern unsigned short vesa_check_vbe(VBEInfoStructure* s);

#endif
