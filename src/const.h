#ifndef __MINI16_CONST
#define __MINI16_CONST

// NOTE: the word at 0000:0600 is reserved for the length of CWD.

#define DMRT_FAR ((char far*)0x00000500)
#define CWD_FAR ((char far*)0x00000602)
#define KERNEL_FAR ((char far*)0x00002000)
#define DISKBUFF_FAR ((char far*)0x0000d000)
#define DISKBUFF_FSTSEC_FAR ((char far*)0x0000d000)
#define DISKBUFF_DIRSEC_FAR ((char far*)0x0000d200)
#define DISKBUFF_CLUSSEC_FAR ((char far*)0x0000d400)

#define DMRT_NEAR ((char*)0x0500)
#define CWD_NEAR ((char*)0x0602)
#define KERNEL_NEAR ((char*)0x2000)
#define DISKBUFF_NEAR ((char*)0xd000)
#define DISKBUFF_FSTSEC_NEAR ((char*)0xd000)
#define DISKBUFF_DIRSEC_NEAR ((char*)0xd200)
#define DISKBUFF_CLUSSEC_NEAR ((char*)0xd400)

#endif
