/* This file contains #define constants for ATA devices*/

#ifndef __ata_h__
#define __ata_h__

#include "BRLNDTYP.H"
#include "TIMERS.H"
#include <string.h>

#define IDE_PRIMARY_IO_BASE 0x1F0
#define IDE_PRIMARY_CONTROL_BASE 0x3F6
#define IDE_SECONDARY_IO_BASE 0x170
#define IDE_SECONDARY_CONTROL_BASE 0x376

#define ATA_IDE_DEVICE_0 0
#define ATA_IDE_DEVICE_1 0x10

/* Source: https://wiki.osdev.org/ATA_PIO_Mode */
#define ATA_DATA_REGISTER 0
#define ATA_ERROR_REGISTER 1
#define ATA_FEATURES_REGISTER 1
#define ATA_SECTOR_COUNT_REGISTER 2
#define ATA_SECTOR_NUMBER_REGISTER 3
#define ATA_LBA_LOW_REGISTER 3
#define ATA_CYLINDER_LOW_REGISTER 4
#define ATA_LBA_MID_REGISTER 4
#define ATA_CYLINDER_HIGH_REGISTER 5
#define ATA_LBA_HIGH_REGISTER 5
#define ATA_DRIVE_REGISTER 6
#define ATA_STATUS_REGISTER 7
#define ATA_COMMAND_REGISTER 7

#define ATA_COMMAND_READ_SECTORS 0x20
#define ATA_COMMAND_IDENTIFY_DEVICE 0xEC

#define ATA_BSY (1<<7)
#define ATA_DRDY (1<<6)
#define ATA_DF (1<<5)
#define ATA_DSC (1<<4)
#define ATA_DRQ (1<<3)
#define ATA_CORR (1<<2)
#define ATA_IDX (1<<1)
#define ATA_ERR 1


/* Word offsets for particular items of interest in the data returned by
 * IDENTIFY_DEVICE */

#define ATA_IDENTIFY_GENERAL_CONFIGURATION 0
#define ATA_IDENTIFY_SERIAL_STRING 10           /* 10 words (words 10..19) */
#define ATA_IDENTIFY_FIMWARE_REVISION 23        /* 4 words (words 23..26) */
#define ATA_IDENTIFY_MODEL_STRING 27            /* 20 words (words 27..46)  */
#define ATA_IDENTIFY_CAPABILITES_1 49           /* Contains flags for LBA and DMA support */
#define ATA_IDENTIFY_MAX_DATA_SET_MANAGEMENT_BLOCKS 105
#define ATA_DATA_SET_MANAGEMENT_COMMAND_SUPPORT 169 /* Contains flags indicating whether TRIM is supported */

typedef struct _ATA_identify_summary_struct{
    /* char arrays for the model, serial number and firmware revision strings + one null char */
    char model [41];
    char serial [21];
    char firmware_ver[9];
    char is_ATA;
    char is_LBA_supported;
    char is_DMA_supported;
    char is_TRIM_supported;
    uint16_t max_data_set_management_blocks;
    uint16_t base_address;
    uint8_t device;
}ATA_identify_summary_t;


/* Structure returned by the IDENTIFY DEVICE command */
/* p.104 of the ATA specification */


/* We only care about a few fields for the purposes of this program:
 * is ATA? word 0, bit 15 = 0 -> ATA device
 * serial number string - words 10..19
 * model string - words 27..46
 * LBA/DMA supported? Word 49 (Capabilities)
 * DMA mode/suport word 63
 * UDMA support word 88 (validity flags in word 53 (uint8 validity_flags_53)
 * TRIM support - word 169
*/
typedef struct _ATA_IDENTIFY_struct{
    uint16_t general_configuration;
    uint16_t obsolete_1;
    uint16_t specific_configuration;
    uint16_t obsolete_3;
    uint16_t retired_4[2];
    uint16_t obsolete_6;
    uint16_t reserved_7[2];
    uint16_t retired_9;
    uint16_t serial_number[10];
    uint16_t retired_20[2];
    uint16_t obsolete_22;
    uint16_t firmware_revision[4];
    uint16_t model_number[20];
    /*The following uint8_t's swapped for endian-ness (word 47, low bits first)*/
    uint8_t drq_sector_limit;
    uint8_t sig_80;
    uint16_t trusted_computing_features;
    uint16_t capabilities;
    uint16_t ext_capabilities;
    uint16_t obsolete_51[2];
    /*The following uint8_t's swapped for endian-ness*/
    uint8_t vailidity_flags_53;
    uint8_t free_fall_control;
    uint16_t obsolete_54[5];
    uint16_t data_security_features;
    uint16_t addressable_sectors_28_bit;
    uint16_t obsolete_62;
    /*The following uint8_t's swapped for endian-ness*/
    uint8_t DMA_mode_support_flags;
    uint8_t DMA_status_flags;
    uint16_t pio_mode_support_flags; /* word 64 */
    uint16_t ignored_1[23]; /* words 65 .. 87 */
    uint16_t udma_modes; /* word 88 */
    uint16_t ignored_2[80]; /* words 89 .. 168 */
    uint16_t data_set_management_support_flags; /* word 169 */
    uint16_t ignored_3[85] /* word 170 .. 254 */
    uint16_t integrity_word;
} ATA_IDENTIFY_out_struct_t;


uint16_t ATA_read_status_reg(uint16_t base);
ATA_identify_summary_t ATA_identify_summary(uint16_t base, uint8_t drive_sel, uint32_t timeout);
int ATA_identify_raw(uint16_t base, uint8_t drive_sel, uint16_t * buffer_512b, uint32_t timeout);
int ATA_LBA_28_PIO_read_absolute(uint16_t base, uint8_t drive_sel, uint32_t lba_addr, uint8_t sect_count, uint16_t * dest_buffer);
int ATA_select_drive(uint16_t base);

#endif
