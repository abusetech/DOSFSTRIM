#include "ATA.H"

uint16_t ATA_read_status_reg(uint16_t base){
    
    /* Reads the status register of the currently selected drive */
    /* This is useful for detecting buses with no connected drives, since they will read
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
     * On error, this function returns a blank (filled with zeros) structure.
     */
    
    int i = 0, j = 0;
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
    
    /* For historical reasons, the characters in the drive identity strings are byte-swapped in each word
     * so each pair of characters in the string needs to be swapped in order to get them into a usable
     * format, hence the following unusual string copy code */
    
    /* Model number string */
    for(i=ATA_IDENTIFY_MODEL_STRING ; i < (ATA_IDENTIFY_MODEL_STRING + 19); i++){
        ident_struct.model[j] = (buffer_512b[i]>>8);
        ident_struct.model[++j] = (buffer_512b[i]&0xFF);
        j++;
    }   
    /* Serial number string */
    j = 0;
    for(i=ATA_IDENTIFY_SERIAL_STRING; i < (ATA_IDENTIFY_SERIAL_STRING + 9); i++){
        ident_struct.serial[j] = (buffer_512b[i]>>8);
        ident_struct.serial[++j] = (buffer_512b[i]&0xFF);
        j++;
    }   
    /* Firmware string */
    j = 0;
    for(i=ATA_IDENTIFY_FIMWARE_REVISION; i < (ATA_IDENTIFY_FIMWARE_REVISION + 3); i++){
        ident_struct.firmware_ver[j] = (buffer_512b[i]>>8);
        ident_struct.firmware_ver[++j] = (buffer_512b[i]&0xFF);
        j++;
    }  
    
    /* The Model, Serial number, and Firmware strings are all space-padded and must be converted to
     * null-terminated strings before use. */
    for(i = 40; i >= 0; i--){
        if (ident_struct.model[i] != 0x20 && ident_struct.model[i] != 0){
            break;
        }else{
            ident_struct.model[i] = 0;
        }
    }
    
    for(i = 20; i >= 0; i--){
        if (ident_struct.serial[i] != 0x20 && ident_struct.serial[i] != 0){
            break;
        }else{
            ident_struct.serial[i] = 0;
        }
    }
    
    for(i = 8; i >= 0; i--){
        if (ident_struct.firmware_ver[i] != 0x20 && ident_struct.firmware_ver[i] != 0){
            break;
        }else{
            ident_struct.firmware_ver[i] = 0;
        }
    }
    
    return ident_struct;
    
}


int ATA_identify_raw(uint16_t base, uint8_t drive_sel, uint16_t * buffer_512b, uint32_t timeout){
    
    /*
     * Executes the ATA_IDENTIFY_DEVICE command on the specified device and fills a buffer with the results
     * base - device base address
     * drive_sel - Selects which device to use on the IDE channel (ATA_IDE_DEVICE_0 / ATA_IDE_DEVICE_1)
     * buffer_512b - Pointer to a 512byte buffer
     * timeout - Timeout in milliseconds. 
     */
    
    int i = 0;
    long timeout_count = 0;
    
    memset(buffer_512b, 0, 512);
    
    /*Wait until the drive isn't busy*/
    if(!timers_wait_until_IO_bit_clear_timeout(base + ATA_DRIVE_STATUS_REGISTER, ATA_BSY, timeout)){
        /* Timeout */
        return 1;
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
    enable();
    
    /*Wait for data*/
    if(!timers_wait_until_IO_bit_set_timeout(base + ATA_DRIVE_STATUS_REGISTER, ATA_DRQ, timeout)){
        /* Timeout */
        /* TODO: Dealing with DF set here would be a good idea 
         * (ERR is set as soon as IDENTIFY_DEVICE is commanded)
         */
        return 1;       
    }
    
    /*Read in the 512 byte block*/
    for (i = 0; i < 256; i++){
        buffer_512b[i] = inport(base);
    }
    
    return 0;
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
    uint8_t drive_sel_reg = 0;
    
    /* invalid device selected */
    if(drive_sel != ATA_IDE_DEVICE_0 && drive_sel != ATA_IDE_DEVICE_1){
        return 1;
    }
    
    /* bit 6 (0x40) set to select LBA mode */
    /* The upper bits of the LBA address are OR'd with the drive select reg */
    drive_sel_reg = 0x40 | drive_sel | ((lba_addr & 0x0F000000) >> 24); 
    /* TODO wait for DRDY with timeout */
    if(!timers_wait_until_IO_bit_clear_timeout(base + ATA_DRIVE_STATUS_REGISTER, ATA_BSY, timeout)){
        /* Timeout */
        return 1;       
    }
    disable();
    outportb(base + ATA_DRIVE_REGISTER, drive_sel_reg);
    outportb(base + ATA_ERROR_REGISTER, 0);
    outportb(base + ATA_SECTOR_COUNT_REGISTER, sect_count);
    outportb(base + ATA_LBA_LOW_REGISTER, (uint8_t)(lba_addr & 0xFF));
    outportb(base + ATA_LBA_MID_REGISTER, (uint8_t)((lba_addr & 0xFF00)>>8));
    outportb(base + ATA_LBA_HIGH_REGISTER, (uint8_t)((lba_addr & 0xFF0000)>>16));
    outportb(base + ATA_COMMAND_REGISTER, ATA_COMMAND_READ_SECTORS)
    enable();
    
    for (block_count = 0; block_count < sect_count; block_count++){
        /* TODO: Checking for ERR / DF here would be wise */
        if(!timers_wait_until_IO_bit_clear_timeout(base + ATA_DRIVE_STATUS_REGISTER, ATA_DRQ, timeout)){
            /* Timeout */
            return 1;       
        }
        for (word_count = 0; word_count < 256; word_count++){
            dest_buffer[word_count + 256 * block_count] = inport(base);
        }
    }
    
    return 0;
}

int ATA_select_drive(uint16_t base, uint8_t drive_sel){
    /* Selects an ATA device on an IDE channel */
    /* Includes a slight delay to allow the device to respond */
    int count = 0;
    outportb(base + ATA_DRIVE_REGISTER, drive_sel);
    /* 14 * 30nS ~= 420nS delay as suggested in https://wiki.osdev.org/ATA_PIO_Mode */
    for(count = 0; count < 14; count++){
        inportb(base + ATA_DRIVE_STATUS_REGISTER);
    }
    
}
