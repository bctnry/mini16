#include "disk.h"

void lba_to_chs(DriveParameter* param,
    unsigned short n,
    char* head,
    char* sector,
    unsigned short* cylinder
) {
    *cylinder = n / (param->max_sector) / (param->max_head + 1);
    *head = n / (param->max_sector) % (param->max_head + 1);
    *sector = n % param->max_sector + 1;
}

void chs_to_lba(DriveParameter* param,
    char head,
    char sector,
    unsigned short cylinder,
    unsigned short* n
) {
    *n = cylinder * (param->max_sector * (param->max_head+1))
        + head * (param->max_sector)
        + cylinder;
}
