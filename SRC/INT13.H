#ifndef __int13_h__
#define __int13_h__

#include "BRLNDTYP.H"

/* http://www.ctyme.com/intr/rb-0708.htm */
/* https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive */

typedef struct _fn42_disk_address_pkt{
    uint8_t pkt_size;
    uint8_t reserved;
    uint16_t blocks_to_transfer;
    uint32_t buffer_addr;
    uint32_t lba_low;
    uint32_t lba_high;
    /*
    uint32_t buffer_address_64_low;
    uint32_t buffer_address_64_high;
    */
} int13_fn42_disk_address_packet_t;


#endif
