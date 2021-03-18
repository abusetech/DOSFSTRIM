#include "BRLNDTYP.H"
#include "PCIBUS.C"
#include <stdio.h>


void print_device_info(uint8_t bus, uint8_t device){
    uint8_t function = 0;
    uint16_t vendor_ID = 0;
    uint16_t device_ID = 0;
    uint8_t class_ID = 0
    vendor_ID = PCI_config_get_vendor_ID(bus, device, function);
    printf("Word 0: %lX \n", PCI_config_read_dword(bus, device, function, 0));
    if (vendor_ID == 0xFFFF){return;}
    device_ID = PCI_config_get_vendor_ID(bus, device, function);
    class_ID = PCI_config_get_class(bus, device, function);
    printf("PCI %d:%d.0 VEN ID %d DEV ID %d CLASS %d\n", bus, device, vendor_ID, device_ID, class_ID);
}

main(){
    uint8_t bus=0;
    uint8_t device=0;
    printf("inportl32: %p outportl32: %p\n", inportl32, outportl32);
    printf("timer port 0x40 read %lX\n", inportl32(0x40));
    printf("ATA IDE  CYL CYL DRV STAT %lX\n", inportl32(0x1F0 + 4));
    printf("Brute force PCI bus scan\n");
    for(bus = 0; bus < 1; bus++){
        for(device = 0; device < 16; device++){
            print_device_info(bus, device);
        }
    }
}
