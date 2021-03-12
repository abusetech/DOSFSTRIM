#include <stdio.h>
#include "ATA.H"
#include "ATA.C"
#include "TIMERS.C"

/* Basic tests */

#define ATA_TIMEOUT 1000

void pretty_print(ATA_identify_summary_t * summary){
    if (!summary->device){
        return;
    }
    printf("ADDR %h DEV %h\n", summary->base_address, summary->device);
    printf("%s FW %s\n", summary->model, summary->serial);
}

main(){
    ATA_identify_summary_t summary;
    
    printf("Identifying Drives.\n");
    /* Primary bus first */
    printf("IDE Primary bus status %#04X\n", ATA_read_status_reg(ATA_PRIMARY_IO_BASE));
     if(!(ATA_read_status_reg(ATA_PRIMARY_IO_BASE) == 0xFFFF)){
         /* Check if *any* devices are on the bus */
          ATA_select_drive(ATA_PRIMARY_IO_BASE, ATA_IDE_DEVICE_0);
          summary = ATA_identify_summary(ATA_PRIMARY_IO_BASE, ATA_IDE_DEVICE_0, ATA_TIMEOUT);
          pretty_print(&summary);
          ATA_select_drive(ATA_PRIMARY_IO_BASE, ATA_IDE_DEVICE_1);
          summary = ATA_identify_summary(ATA_PRIMARY_IO_BASE, ATA_IDE_DEVICE_1, ATA_TIMEOUT);
          pretty_print(&summary);
     }
     
     printf("IDE secondary bus status %#04X\n", ATA_read_status_reg(ATA_SECONDARY_IO_BASE));
      if(!(ATA_read_status_reg(ATA_SECONDARY_IO_BASE) == 0xFFFF)){
         /* Check if *any* devices are on the bus */
          ATA_select_drive(ATA_SECONDARY_IO_BASE, ATA_IDE_DEVICE_0);
          summary = ATA_identify_summary(ATA_SECONDARY_IO_BASE, ATA_IDE_DEVICE_0, ATA_TIMEOUT);
          pretty_print(&summary);
          ATA_select_drive(ATA_SECONDARY_IO_BASE, ATA_IDE_DEVICE_1);
          summary = ATA_identify_summary(ATA_SECONDARY_IO_BASE, ATA_IDE_DEVICE_1, ATA_TIMEOUT);
          pretty_print(&summary);
     }
}
