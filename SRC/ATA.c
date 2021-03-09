#include "ATA.h"

uint16_t ATA_read_status_reg(uint16_t base){
    
    /* Reads the status register of the currently selected drive */
    /* This is useful for detecting busses with no connected drives, since they will read
     * 0xFFFF (invalid).
     */
    
    return inport(base + ATA_STATUS_REGISTER);
}


ATA_identify_summary_t ATA_identify_summary(uint16_t base, uint8_t drive_sel, uint32_t timeout){
    
    /*
     * Executes the ATA_IDENTIFY command on the specified device and fills the members of a 
     * ATA_identify_summary_t struct (See ATA.h) with the results.
     * 
     * base - device base address
     * drive_sel - Selects which device to use on the IDE channel (ATA_IDE_DEVICE_0 / ATA_IDE_DEVICE_1)
     * 
     */
    
    /* Struct containing the results */
    ATA_identify_summary_t ident_struct;
    /* allocate a buffer to write the status information into */
    uint16_t buffer[256];
    /* Clear buffers and structs */
    memset(&ident_struct, 0, sizeof(ATA_identify_summary_t));
    memset(buffer, 0, 512);
    /* IDENTIFY_DEVICE */
    if (!ATA_identify_raw(base, drive_sel, buffer, timeout)){
        /* return a blank structure on error */
        return ident_struct;
    }
    
    /* First, preform some sanity checks on the result */
    /* Bits 15:8 of word 47 should ALWAYS be 80h (p. 104, ATA/ATAPI command set 3) */
    if (buffer[47] >> 8 != 0x80){
        return ident_struct;
    }
    
    /* IDENTIFY_DEVICE is a 28bit LBA command, so the LBA flag of CAPABILITIES_1 (Word 49) must be set */
    if ((buffer[ATA_IDENTIFY_CAPABILITIES_1] & (1 << 9)) == 0){
        return ident_struct;
    }
    
    ident_struct.base_address = base;
    ident_struct.device = drive_sel;
    /* Bit 15 of GENERAL_CONFIGURATION clear => ATA Device */ 
    ident_struct.is_ATA = (buffer[ATA_IDENTIFY_GENERAL_CONFIGURATION] & 0x8000 == 0) ? 0 : 1;
    /* LBA28 must be supported to get to this point */
    ident_struct.is_LBA_supported = 1;
    /* DMA support is indicated by a flag in the CAPABILITIES_1 word */
    ident_struct.is_DMA_supported = (buffer[ATA_IDENTIFY_CAPABILITIES_1] & (1 << 9)) > 0 ? 1 : 0;
    /* TRIM support is indicated by a flag in the DATA_SET_MANAGEMENT word */
    ident_struct.is_TRIM_SUPPORTED = buffer[ATA_DATA_SET_MANAGEMENT_COMMAND_SUPPORT] & 1;
    /* How many blocks can be TRIM'd at once? */
    ident_struct.max_data_set_management_blocks = buffer[ATA_IDENTIFY_MAX_DATA_SET_MANAGEMENT_BLOCKS];
    
}


int ATA_identify_raw(uint16_t base, uint8_t drive_sel, uint16_t * buffer_512b, uint32_t timeout){
    
    /*
     * Executes the ATA_IDENTIFY_DEVICE command on the specified device and fills a buffer with the results
     * base - device base address
     * drive_sel - Selects which device to use on the IDE channel (ATA_IDE_DEVICE_0 / ATA_IDE_DEVICE_1)
     * buffer_512b - Pointer to a 512byte buffer
     * timeout - Upper count to use for the spin lock. Interrupts need to be disabled for this command to run,
     * so using the system timers isn't possible. Set this to a large value if you simply want to ignore it
     */
    
    int i = 0;
    long timeout_count = 0;
    
    memset(buffer_512b, 0, 512);
    
    /*Wait until the drive isn't busy*/
    while(get_ide_status(base) & BSY){
        timeout_count++;
        if (timeout_count > timeout){return 0;}
    }
    /*disable interrupts*/
    disable();
    /* Set LBA, sector count registers to zero */
    outportb(base + ATA_SECTOR_COUNT_REGISTER, 0);
    outportb(base + ATA_LBA_LOW_REGISTER, 0);
    outportb(base + ATA_LBA_MID_REGISTER, 0);
    outportb(base + ATA_LBA_HIGH_REGISTER, 0);
    /*Select which IDE device to query*/
    outportb(base + ATA_DRIVE_REGISTER, drive_sel); /*0 selects dev 0, 0x10 selects device 1*/
    /*Write the command to the register - this sends the command to the drive*/
    outportb(base + ATA_COMMAND_REGISTER, ATA_COMMAND_IDENTIFY_DEVICE); /*IDENTIFY_DEVICE*/
    /*Wait for data*/
    timeout_count = 0;
    while(get_ide_status(base) & DRQ == 0){
        timeout_count++;
        if (timeout_count > timeout >> 1){
            enable();
            return 0;
        } 
    }
    /*Read in the 512 byte block*/
    for (i = 0; i < 256; i++){
        buffer_512b[i] = inport(base);
    }
    enable();
    return 1;
}

int ATA_LBA_28_PIO_read_absolute(uint16_t base, uint8_t drive_sel, uint32_t lba_addr, uint8_t sect_count, uint16_t * dest_buffer){
    
    /*
     * 28bit Programmed I/O Read directly from ATA device
     * base - device base address
     * drive_sel - Selects which device to use on the IDE channel (ATA_IDE_DEVICE_0 / ATA_IDE_DEVICE_1)
     * lba_addr 28bit LBA address
     * sect_count - number of sectors to transfer
     * dest_bufer - Output buffer. Must be 512 * sect_count bytes large.
     */
    
    int word_count = 0;
    int block_count = 0;
    
    /* invalid device selected */
    if(drive_sel != ATA_IDE_DEVICE_0 || drive_sel != ATA_IDE_DEVICE_1){
        return 1;
    }
    
    /* bit 6 (0x40) set to select LBA mode */
    /* The upper bits of the LBA address are OR'd with the drive select reg */
    uint8_t drive_sel_reg = 0x40 | drive_sel | ((lba_addr & 0x0F000000) >> 24); 
    
    outportb(base + ATA_DRIVE_REGISTER, drive_sel_reg);
    outportb(base + ATA_ERROR_REGISTER, 0);
    outportb(base + ATA_SECTOR_COUNT_REGISTER, sect_count);
    outportb(base + ATA_LBA_LOW_REGISTER, (uint8_t)(lba_addr & 0xFF));
    outportb(base + ATA_LBA_MID_REGISTER, (uint8_t)((lba_addr & 0xFF00)>>8));
    outportb(base + ATA_LBA_HIGH_REGISTER, (uint8_t)((lba_addr & 0xFF0000)>>16));
    outportb(base + ATA_COMMAND_REGISTER, ATA_COMMAND_READ_SECTORS)
    
    for (block_count = 0; block_count < sect_count; block_count++){
        for (word_count = 0; word_count < 256; word_count++){
            dest_buffer[word_count + 256 * block_count] = inport(base);
        }
    }
    
    return 0;
}
