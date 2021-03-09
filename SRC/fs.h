#ifndef __fs_h__
#define __fs_h__

#include "borland_types.h"

typedef struct _MBR_partition_entry{
    uint8_t drive_attributes;
    uint8_t c_start, h_start, s_start;
    uint8_t partition_type;
    uint8_t c_end, h_end, s_end;
    uint32_t partition_start_LBA;
    uint32_t sectors_in_partition;
} MBR_partition_entry_t;

typedef struct _disk_MBR{
    uint8_t MBR_bootstrap [440];
    uint32_t disk_id;
    uint16_t reserved_1bc;
    MBR_partition_entry_t partition_entries[4];
    uint16_t bootsector_signature;
} disk_MBR_t;

typedef struct _FAT16_EBPB{
    uint8_t drive_number;
    uint8_t nt_flags;
    /*signature must be 0x28 or 0x29 to be FAT16*/
    uint8_t signature;
    uint32_t volume_ID;
    uint8_t volume_label[11];
    uint8_t system_identifier[8];
    uint8_t boot_code[448];
    uint16_t partition_signature;
} FAT16_EBPB_t;

typedef struct _FAT32_EBPB{
    uint32_t sectors_per_FAT;
    uint16_t flags;
    uint16_t FAT_version;
    uint32_t root_dir_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_root_sector;
    uint8_t reserved_034[16];
    uint8_t drive_number;
    uint8_t nt_flags;
    /*Must be 0x28 or 0x29*/
    uint8_t signature;
    uint32_t volume_ID;
    uint8_t volume_label[11];
    uint8_t system_identifier[8];
    uint8_t boot_code[420];
    uint16_t partition_signature;
} FAT32_EBPB_t;

typedef union _FAT_EBPB{
    FAT16_EBPB_t FAT16_EBPB;
    FAT32_EBPB_t FAT32_EBPB;
} FAT_EBPB_t;

typedef struct _FAT_BPB{
    uint8_t jmp_short[3];
    uint8_t OEM_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t number_of_FATs;
    uint16_t directory_entry_count;
    uint16_t sectors_in_volume;
    uint8_t media_descriptor;
    uint16_t sectors_per_FAT;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sector_count;
    uint32_t large_sector_count;
    FAT_EBPB_t EBPB;
} FAT_BPB_t;

/* FAT Boot record, should be exactly 512 bytes*/
typedef struct _FAT_BR{
    FAT_BPB_t BPB;
    FAT_EBPB_t EBPB;
} FAT_BR_t;

typedef struct _FAT32_FSInfo{
    uint32_t lead_signature;    /*  0x41615252 */
    uint8_t reserved_1ec[480];
    uint32_t middle_signature;  /* 0x61417272 */
    uint32_t free_cluster_count;
    uint32_t first_known_free_cluster;
    uint8_t reserved_1f0[12];
    uint32_t trail_signature;   /* 0xAA550000 */
} FAT32_FSInfo_t;

#endif
