#include "ASMHLP.H"
#include "BRLNDTYP.H"

/* PCI Bus enumeration code */

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

/* Elements common to all PCI config header types*/
typedef struct __PCI_config_header_common{
    uint16_t device_id, vendor_id, status, command;
    uint8_t class_code, subclass, prog_if, rev_id;
    uint8_t bist, header_typ, latency_timer, cache_lane_typ;
} PCI_config_header_common__t

/* The following two functions are based on code an information from the osdev wiki
 * https://wiki.osdev.org/PCI
 */

uint32_t PCI_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg){
    uint32_t address = 0;
    /* The first bit is a flag that tells the PCI bus to update the value at CONFIG_DATA */
    /* The register to be addressed in the configuration space must be double word (32bit) aligned, so the last two bits must be zero*/
    address = 0x80000000L | ((uint32_t)bus)<<16 | ((uint32_t)device & 0b111111)<<11 | ((uint32_t)func)<<8 | reg &0xFC;
    /* it might be possible to use 16 bit reads here 
       ie. outport(PCI_CONFIG_ADDRESS + 2, address & 0xFFFF);
           outport(PCI_CONFIG_ADDRESS, address >> 16); 
     */
    outportl32(PCI_CONFIG_ADDRESS, address);
    return inportl32(PCI_CONFIG_DATA);
}

uint16_t PCI_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg){
    uint32_t address = 0;
    /* The first bit is a flag that tells the PCI bus to update the value at CONFIG_DATA */
    /* The register to be addressed in the configuration space must be double word (32bit) aligned, so the last two bits must be zero*/
    address = 0x80000000L | ((uint32_t)bus)<<16 | ((uint32_t)device & 0b111111)<<11 | ((uint32_t)func)<<8 | reg &0xFC;
    /* it might be possible to use 16 bit reads here 
       ie. outport(PCI_CONFIG_ADDRESS + 1, address & 0xFFFF);
           outport(PCI_CONFIG_ADDRESS, address >> 16); 
     */
    outportl32(PCI_CONFIG_ADDRESS, address);
    /* If bit two is set in reg, return the lower half of the double word*/
    /* Otherwise, return the upper half of the word */
    return (inportl32(PCI_CONFIG_DATA) >> ((reg & 2) << 3)) & 0xFFFF;
    /*This could also be accomplished using two separate inport() calls */
    /*
     * if (reg & 2){
     * return inport(PCI_CONFIG_DATA + 2);
     * }else{
     * return inport(PCI_CONFIG_DATA);
     * }
     */
}

