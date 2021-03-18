#include "ASMHLP.H"
#include "BRLNDTYP.H"

/* PCI Bus enumeration code */

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_CONFIG_DEVICE_ID_BYTE_OFFSET 0x00
#define PCI_CONFIG_VENDOR_ID_BYTE_OFFSET 0x02
#define PCI_CONFIG_CLASS_BYTE_OFFSET 0x08
#define PCI_CONFIG_SUBCLASS_BYTE_OFFSET 0x09
#define PCI_CONFIG_HEADER_TYPE_BYTE_OFFSET 0x0D

/* Elements common to all PCI config header types*/
typedef struct __PCI_config_header_common{
    uint16_t device_id, vendor_id, status, command;
    uint8_t class_code, subclass, prog_if, rev_id;
    uint8_t bist, header_typ, latency_timer, cache_lane_typ;
} PCI_config_header_common__t;


/* 
 * The following two functions are based on code and information from the osdev wiki
 * https://wiki.osdev.org/PCI
 */

uint32_t PCI_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg){
    uint32_t lbus, ldevice, lfunction;
    uint32_t address = 0, read = 0;
    
    lbus = bus;
    ldevice = device;
    lfunction = function;
    /* The first bit is a flag that tells the PCI bus to update the value at CONFIG_DATA */
    /* The register to be addressed in the configuration space must be double word (32bit) aligned, so the last two bits must be zero*/
    address =   0x80000000l |                           \
                (lbus<<16) |                            \
                (ldevice<<11) |                         \
                (lfunction<<8) |                        \
                (reg&0xFC);    
    /* it might be possible to use 16 bit reads here 
       ie. outport(PCI_CONFIG_ADDRESS, address & 0xFFFF);
           outport(PCI_CONFIG_ADDRESS+2, address >> 16); 
     */

    printf("ADDR %lX ", address);
    disable();
    outportl32(PCI_CONFIG_ADDRESS, address);
    read = inportl32(PCI_CONFIG_DATA);
    enable();
    return read;
}

uint16_t PCI_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg){
    uint32_t pci_dword = 0;
    pci_dword = PCI_config_read_dword(bus, device, function, reg);
    return (pci_dword>>((reg & 0x02) * 8))&0xFFFF;
}

uint8_t PCI_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg){
    /*
     * 
     * Reads a byte from the PCI configuration space. This function is slow and should be avoided
     * unless you *really* only need one byte.
     * 
     */
    uint16_t word = 0;
    word = PCI_config_read_word(bus, device, function, reg & 0xFE);
    if(reg & 1 == 0){
        return word >> 8;
    }else{
        return (word & 0xFF);
    }
}

uint8_t PCI_config_get_class(uint8_t bus, uint8_t device, uint8_t function){
    return PCI_config_read_byte(bus, device, function, PCI_CONFIG_CLASS_BYTE_OFFSET);
}

uint8_t PCI_config_get_subclass(uint8_t bus, uint8_t device, uint8_t function){
    return PCI_config_read_byte(bus, device, function, PCI_CONFIG_SUBCLASS_BYTE_OFFSET);
}

uint8_t PCI_config_get_vendor_ID(uint8_t bus, uint8_t device, uint8_t function){
    return PCI_config_read_word(bus, device, function, PCI_CONFIG_VENDOR_ID_BYTE_OFFSET);
}

uint8_t PCI_config_get_device_ID(uint8_t bus, uint8_t device, uint8_t function){
    return PCI_config_read_word(bus, device, function, PCI_CONFIG_DEVICE_ID_BYTE_OFFSET);
}
