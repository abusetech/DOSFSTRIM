#include "int13.h"
#include <string.h>
#include <dos.h>

/* https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive */
int int13_extended_read(void * buffer, uint8_t drive_number, int blocks, uint32_t lba_low, uint32_t lba_high){
    /* BIOS INT13 Function 0x42 disk extended read */
    /* Buffer is some multiple of 512 bytes, blocks is the number of 512 byte blocks that should be transferred*/
    int13_fn42_disk_address_packet_t pkt;
    /* zero out the buffers */
    memset(buffer, 0, (long)512 * blocks);
    memset(&pkt, 0, sizeof(int13_fn42_disk_address_packet_t));  
    
    pkt.pkt_size = sizeof(int13_fn42_disk_address_packet_t);
    pkt.blocks_to_transfer = blocks;
    /* TODO: is this correct? segment * 16 + offset.*/
    /* Offset declared first for endian-ness */
    pkt.buffer_addr = ((uint32_t)FP_OFF(buffer) << 16) | FP_SEG(buffer); 
    pkt.lba_low = lba_low;
    pkt.lba_high = lba_high;
    
    union REGS regs;
    /* function 0x42 */
    regs.h.ah = 0x42;
    /* drive number */
    regs.h.dl = drive_number;
    /* parameter packet address */
    regs.h.ds = FP_SEG(&pkt);
    regs.h.si = FP_OFF(&pkt);
    
    /* save the current segment:offset regs */
    struct SREGS segregs;
    segread(&segregs);
    
    /* call the interrupt routine, overwriting the registers */
    int86x(0x13, &regs, &regs, &segregs);
    
    /* check for an error */
    if (x.cflag != 0){
        return x.cflag;
    }
    return 0;
}
