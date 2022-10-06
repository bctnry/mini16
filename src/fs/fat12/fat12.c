#include "../../const.h"
#include "fat12.h"
#include "../../disk/disk.h"
#include "../../term/term.h"
#include "../../util/debug.h"

// load fat table (group n) (n starts from 0)
// 0.  require a DriveParameter(dp) and a FATDescriptor(desc).
// 1.  get the start LBA:
//         desc.n_reserved_sector    // reserved region.
//         + (desc.n_root_entry * 32 / desc.bytes_per_sector)    // root directory
//         + n * 3
// 2.  calculate CHS of lba_start with dp (c, h, s)
// 3.  load how many sectors this time?
//     how many groups (n_total):
//         floor(desc.sector_per_fat / 3) + (desc.sector_per_fat % 3 != 0)
//     so if n == n_total-1 we should load n_total % 3; otherwise we should load 3.
// NOTE THAT always load a multiple of 3 sectors for simpler FAT12 support, because
// FAT12 cluster does not fit nicely within the common 512 byte sectors goddamnit
// from now on FAT tables are divided into groups of 3.
// NOTE: FAT are loaded at DISKBUFF_CLUSSEC_FAR.
char fat12_load_fat_table(
        struct DriveParameter far* dp,
        struct FATDescriptor far* desc,
        unsigned short n
) {
    unsigned short start_lba;
    unsigned short n_total;   // n_total but floor instead of ceil.
    unsigned short cylinder;
    char load_size, head, sector;

    start_lba =
        desc->n_reserved_sector
        + (desc->n_root_entry * 32 / desc->bytes_per_sector)
        + n * 3;
    n_total = desc->sector_per_fat / 3 + (desc->sector_per_fat % 3? 0 : 1);
    load_size = n == n_total-1? n_total % 3 : 3;
    lba_to_chs(dp, start_lba, &head, &sector, &cylinder);
    return read_sector(
        DISKBUFF_CLUSSEC_FAR,
        load_size,
        dp->id,
        head,
        cylinder,
        sector
    );
}


// calculate the group id of the cluster_id
// 0.  require a FATDescriptor(desc)
// 1.  get cluster count per 3 sector (n_cluster_per_group).
//         desc.bytes_per_sector * 3 / 12
//         --> desc.bytes_per_sector / 4
// 2.  return cluster_id / n_cluster_per_group
//         --> cluster_id / (desc.bytes_per_sector / 4)
unsigned short fat12_groupid_of_cluster(
        struct FATDescriptor far* desc,
        unsigned short cluster_id
) {
    return cluster_id / (desc->bytes_per_sector / 4);
}

// cluster-id to lba
// 0.  require a DriveParameter(dp)
//         and a FATDescriptor(desc)
// 1.  calculate the start of the data region on disk; this is where the
//     cluster-id maps to.
//         desc.n_reserved_sector    // reserved sector.
//         + desc.sector_per_fat * desc.n_fat    // FAT region.
//         + (desc.n_root_entry * 32 / desc.bytes_per_sector)  // root directory
//     this is the base of data region. (data_base).
// 2.  calculate the mapped lba:
//         data_base + cluster_id * desc.sector_per_cluster
// NOTE: unsigned short will restrict the supported size to ~32MBytes, but FAT12 pretty
// much only exists on floppy disks anyway.
unsigned short fat12_cluster_to_lba(
        struct DriveParameter far* dp,
        struct FATDescriptor far* desc,
        unsigned short cluster_id
) {
    unsigned short data_base =
        desc->n_reserved_sector
        + desc->sector_per_fat * desc->n_fat
        + (desc->n_root_entry * 32 / desc->bytes_per_sector);
    return data_base
        + cluster_id * desc->sector_per_cluster;
}

// load cluster into memory by cluster-id
// 0.  require a DriveParameter(dp)
//             a FATDescriptor(desc)
//         and a char far*(target)
// 1.  calculates the lba of cluster_id with dp and desc
// 2.  calculate chs from lba with dp
// 3.  read the sector(s) into target with desc
//         (because somehow we can have cluster size more than 1 sector.)
char fat12_load_cluster(
        struct DriveParameter far* dp,
        struct FATDescriptor far* desc,
        char far* target,
        unsigned short cluster_id
) {
    char head, sector, i, n_sector_per_cluster;
    unsigned short cylinder, lba;
    n_sector_per_cluster = desc->sector_per_cluster;
    lba = fat12_cluster_to_lba(dp, desc, cluster_id);
    lba_to_chs(dp, lba, &head, &sector, &cylinder);
    return read_sector(
        target,
        n_sector_per_cluster,
        dp->id,
        head,
        cylinder,
        sector
    );
}

// get the cluster value of a cluster id.
// 0.  require a DriveParameter(dp)
//             a FATDescriptor(desc)
//         and a FATClusterPointer(cp)
// a.  cp is pre-loaded with the cluster_id; its current_value
//     field will be set upon returning.
// 1.  calculate the group id with fat12_groupid_of_cluster
// 2.  load the group into DISKBUFF_CLUSSEC_FAR
// 3.  read the value.
void fat12_get_cluster_value_of(
        struct DriveParameter far* dp,
        struct FATDescriptor far* desc,
        struct FATClusterPointer far* cp
) {
    unsigned short modded_id;
    unsigned short group = fat12_groupid_of_cluster(desc, cp->cluster_id);
    fat12_load_cluster(dp, desc, DISKBUFF_CLUSSEC_FAR, group);
    modded_id = cp->cluster_id % (desc->bytes_per_sector / 4);
    modded_id = modded_id / 2 * 3;
    cp->cluster_value = (
        modded_id % 2?
            ((unsigned short)DISKBUFF_CLUSSEC_FAR[modded_id+1])<<4|DISKBUFF_CLUSSEC_FAR[modded_id]
        :
            ((unsigned short)DISKBUFF_CLUSSEC_FAR[modded_id+2])<<8|DISKBUFF_CLUSSEC_FAR[modded_id+1]
    );
}

// find next cluster with a FATClusterPointer
// NOTE: FAT values are pointers pointing to the next cluster
// 0.  require a DriveParameter(dp)
//             a FATDescriptor(desc)
//         and a FATClusterPointer(cp)
// 1.  calculate how many cluster there is in 3 sector (n_cluster_in_group):
//         (3 * desc.bytes_per_sector) / 12
// 2.  calculate which group is cp.cluster_id in (group_current)
// 3.  calculate which group is cp.cluster_value in (group_next)
// 4.  if group_current != group_next:
//         load fat table (group group_n)
// 5.  calculate the offset
//     because we are talking about fat12 here so things are trickier. for fat12
//     the clusters are arranged like this:
//             AB CD EF GH IJ KL ...
//         three bytes maps to 2 cluster & the whole thing is in little-endian,
//         so it's actually:
//             cluster 1 & 0: EFCDAB --> 0xEFC 0xDAB
//                 cluster 0: offset 1 lower nibble ~ offset 0 byte
//                 cluster 1: offset 2 byte ~ offset 1 higher nibble
//             cluster 3 & 2: KLIJGH --> 0xKLI 0xJGH
//                 cluster 2: offset 4 lower nibble ~ offset 3 byte
//                 cluster 1: offset 5 byte ~ offset 4 higher nibble
//     we should make next_cluster_id be next_cluster_id % length_of_group_in_cluster first.
//     let modded_id = next_cluster_id % (desc.bytes_per_sector * 3 / 12)
//     so for even cluster id:
//         offset [modded_id / 2 * 3 + 1] lower nibble ~ offset [modded_id / 2 * 3] byte
//     for odd cluster id:
//         offset [modded_id / 2 * 3 + 2] byte ~ offset [modded_id / 2 * 3 + 1] higher nibble
// 6.  retrieve the cluster value.
// 7.  update cp.
// RETURNS 1 and don't do anything if end of cluster chain.
char fat12_next_cluster(
        struct DriveParameter far* dp,
        struct FATDescriptor far* desc,
        struct FATClusterPointer far* cp
) {
    unsigned short n_cluster_in_group;
    unsigned short group_current, group_next;
    unsigned short modded_id;
    unsigned short next_cluster_id;
    unsigned short next_cluster_value;

    switch (cp->cluster_value) {
        case 0x000: case 0x001: case 0xff0:
        case 0xff7: { return 1; }
        default: {
            if (
                (0xff8 <= cp->cluster_value && cp->cluster_value <= 0xfff)
                || (0xff0 <= cp->cluster_value && cp->cluster_value <= 0xff5)
            ) {
                return 1;
            }
        }
    }

    next_cluster_id = cp->cluster_value;
    n_cluster_in_group = (3 * desc->bytes_per_sector) / 12;
    group_current = fat12_groupid_of_cluster(desc, cp->cluster_id);
    group_next = fat12_groupid_of_cluster(desc, next_cluster_id);
    if (group_current != group_next) {
        fat12_load_cluster(
            dp,
            desc,
            DISKBUFF_CLUSSEC_FAR,
            group_next
        );
    }
    modded_id = next_cluster_id % (desc->bytes_per_sector / 4);
    modded_id = modded_id / 2 * 3;
    next_cluster_value = (
        modded_id % 2?
            ((unsigned short)DISKBUFF_CLUSSEC_FAR[modded_id+1])<<4|DISKBUFF_CLUSSEC_FAR[modded_id]
        :
            ((unsigned short)DISKBUFF_CLUSSEC_FAR[modded_id+2])<<8|DISKBUFF_CLUSSEC_FAR[modded_id+1]
    );
    cp->cluster_id = next_cluster_id;
    cp->cluster_value = next_cluster_value;
    return 0;
}

// filename match - match "8.3" with space-padded 11.
// 0.  require a char far* (subj)
//         and a FATDirectoryEntry far* (obj)
// a.  return 0 on success, 1 on fail.
// 1.  if obj is dir (have size of 0):
//         only match name & return.
//     or else:
// [A] Match file name.
// a.  i = 0
// 1.  while subj[i] and subj[i] != '.'
//             and i < 8 and obj.name[i] != ' '
//             and subj[i] == obj.name[i]:
//         i++
// 2.  if not (!subj[i] or subj[i] == '.')    // end of normal name
//             and ((i < 8 and obj.name[i] == ' ') or i >= 8) // end of obj normal name
//         return 1.
// [B] Match extname phase
// 1.  if subj[i] == '.': i++
//     if !subj[i]:
//         if obj.name[8] == ' ': return 0 
//         else: return 1
//     j = 0
// 2.  while subj[i]
//             and j < 3 and obj.ext_name[j] != ' '
//             and subj[i] == obj.ext_name[j]:
//         i++, j++
// 3.  if !subj[i] and ((j < 3 and obj.ext_name[j] == ' ') or j >= 3): return 0
//     else return 1
char fat12_match_filename(
        char far* subj,
        struct FATDirectoryEntry far* obj
) {
    char i = 0, j = 0;
    
    // if directory, only match first name.
    if (obj->file_size == 0) {
        while (subj[i] && i < 8 && subj[i] == obj->name[i]) {
            i++;
        }
        return !subj[i]? 0 : 1;
    }

    while (
        subj[i] && subj[i] != '.'
        && obj->name[i] && obj->name[i] != ' '
        && subj[i] == obj->name[i]
    ) {
        i++;
    }
    if (!((!subj[i] || (subj[i] == '.')) && (i >= 8 || obj->name[i] == ' '))) {
        return 1;
    }
    if (subj[i] == '.') { i++; }
    if (!subj[i]) {
        return obj->name[8] == ' '? 0 : 1;
    }
    j = 0;
    while (
        subj[i]
        && (j < 3 && obj->ext_name[j] != ' ')
        && (subj[i] == obj->ext_name[j])
    ) {
        i++;
        j++;
    }
    if (!subj[i] && ((j < 3 && obj->ext_name[j] == ' ') || j >= 3)) {
        return 0;
    } else {
        return 1;
    }
}

// go over a directory
// 0.  require a DriveParameter(dp)
//             a FATDescriptor(desc)
//         and a FATClusterPointer(cp) which refers to the dir file's starting point.
// a.  sector_i = 0  // for counting pages
//     page_i = 0    // for counting inside pages
//     i = 0         // for counting total should never be
// b.  cp == 0 means go over the root directory.
// [A] Loading phase
// 1.  if it's root dir, get root dir lba:
//         desc.n_reserved_sector   // reserved region.
//         + desc.n_fat * desc.sector_per_fat      // FAT region.
//     or else, get cp pointed lba with fat12_cluster_to_lba.
// 2.  load a sector with cp at DISKBUFF_DIRSEC_FAR.
// [B] Page handling phase
// 1.  for entries in DISKBUFF_DIRSEC_FAR:
//         if an empty entry is encountered:
//             break out of <for> immediately and skip Phase C.
//         do stuff with entry.
//         i++
//         page_i++
// [C] Check-if-need-to-load-next phase
// 1.  if it's root dir:
//         if i < max_root_dir_entry, then it's needed.
//     if it's not root dir:
//         if next_cluster(cp) returns 0, then it's needed.
//     otherwise it's not needed.
// 2.  if it's not needed to load next, skip Phase D.
// [D] Load-next phase
// 1.  if it's root dir:
//         sector_i++
//         load next root dir sector to DISKBUFF_DIRSEC_FAR.
//     if it's not root dir (i.e. cluster-chain):
//         because we already moved cp to the next cluster if it's ever needed,
//         we directly load new sector with cp.
// 2.  goto Phase B.
void fat12_iterate_over_dir(
        struct DriveParameter far* dp,
        struct FATDescriptor far* desc,
        struct FATClusterPointer far* cp,
        // return 1 if the iteration should not go further.
        char(*f)(struct FATDirectoryEntry far* x)
) {
    char root = (cp == 0? 1 : 0);
    unsigned short sector_i, page_i, i;
    unsigned short root_lba;
    unsigned short cylinder;
    char head, sector;
    unsigned short page_bound;
    unsigned short max_root_dir_entry;
    char should_stop = 0;
    FATDirectoryEntry far* dir_entry;

    sector_i = 0; page_i = 0; i = 0;
    page_bound = desc->bytes_per_sector / 32;

    // Phase_A:        // [A]
    max_root_dir_entry = desc->n_root_entry;
    root_lba =
        root? desc->n_reserved_sector + desc->n_fat * desc->sector_per_fat
        : fat12_cluster_to_lba(dp, desc, cp->cluster_id);
    lba_to_chs(dp, root_lba, &head, &sector, &cylinder);
    
    read_sector(
        DISKBUFF_DIRSEC_FAR,
        1,
        dp->id,
        head,
        cylinder,
        sector
    );
    
    Phase_B:        // [B]
    page_i = 0;
    dir_entry = ((FATDirectoryEntry far*)DISKBUFF_DIRSEC_FAR);
    while (page_i < page_bound) {
        if (!dir_entry->name[0]) {
            goto End;
        }
        should_stop = (*f)(dir_entry);
        if (should_stop) { goto End; }
        i++;
        page_i++;
        dir_entry++;
    }

    // Phase_C:        // [C]
    if (!(
        (root & (i < max_root_dir_entry))
        || (!root & fat12_next_cluster(dp, desc, cp) == 0)
    )) { goto End; }
    
    // Phase_D:        // [D]
    if (root) {
        sector_i++;
        lba_to_chs(dp, root_lba+sector_i, &head, &sector, &cylinder);
        read_sector(DISKBUFF_DIRSEC_FAR, 1, dp->id, head, cylinder, sector);
    } else {
        fat12_load_cluster(
            dp,
            desc,
            DISKBUFF_CLUSSEC_FAR,
            cp->cluster_id
        );
        page_i = 0;
    }
    goto Phase_B;

    End: return;
}


// find entry in directory
// same as go over a directory, but:
//    if entry found, set cp to the start of cluster, and skip Phase C & D.
// return 0 on not found.
FATDirectoryEntry far* fat12_find_entry_in_directory(
        struct DriveParameter far* dp,
        struct FATDescriptor far* desc,
        struct FATClusterPointer far* cp,
        char far* subj
) {
    char root = (cp == 0? 1 : 0);
    unsigned short sector_i, page_i, i;
    unsigned short root_lba;
    unsigned short cylinder;
    char head, sector;
    unsigned short page_bound;
    unsigned short max_root_dir_entry;
    char found = 0;
    FATDirectoryEntry far* dir_entry;

    sector_i = 0; page_i = 0; i = 0;
    page_bound = desc->bytes_per_sector / 32;

    // Phase_A:        // [A]
    max_root_dir_entry = desc->n_root_entry;
    root_lba =
        root? desc->n_reserved_sector + desc->n_fat * desc->sector_per_fat
        : fat12_cluster_to_lba(dp, desc, cp->cluster_id);
    lba_to_chs(dp, root_lba, &head, &sector, &cylinder);
    read_sector(
        DISKBUFF_DIRSEC_FAR,
        1,
        dp->id,
        head,
        cylinder,
        sector
    );
    
    Phase_B:        // [B]
    page_i = 0;
    dir_entry = ((FATDirectoryEntry far*)DISKBUFF_DIRSEC_FAR);
    while (page_i < page_bound) {
        if (!dir_entry->name[0]) {
            found = 0; goto End;
        }
        // TODO: the match is buggy. fix that.
        if (fat12_match_filename(subj, dir_entry) == 0) {
            found = 1; goto End;
        }
        i++;
        page_i++;
        dir_entry++;
    }

    // Phase_C:        // [C]
    if (!(
        (root & (i < max_root_dir_entry))
        || (!root & fat12_next_cluster(dp, desc, cp) == 0)
    )) { goto End; }
    
    // Phase_D:        // [D]
    if (root) {
        sector_i++;
        lba_to_chs(dp, root_lba+sector_i, &head, &sector, &cylinder);
        read_sector(DISKBUFF_DIRSEC_FAR, 1, dp->id, head, cylinder, sector);
    } else {
        fat12_load_cluster(
            dp,
            desc,
            DISKBUFF_CLUSSEC_FAR,
            cp->cluster_id
        );
        page_i = 0;
    }
    goto Phase_B;

    End:
    if (!found) { return (FATDirectoryEntry far*)0; }
    else {
        cp->cluster_id = dir_entry->start_of_file;
        fat12_get_cluster_value_of(dp, desc, cp);
        return dir_entry;
    }
}

