/* The following code is used for testing. This file was created while I was playing with this idea and will not be present in the finished application */

#include <DOS.H>
#include <time.h>
#include <stdio.h>
#include <string.h>

#define IDE_PRIMARY_1 0x1F0
#define IDE_SECONDARY_1 0x170

#define IDE_DATA 0
#define IDE_ERROR 1
#define IDE_FEATURES 1
#define IDE_SECTOR_COUNT 3
#define IDE_LBA_1 4
#define IDE_LBA_2 5
#define IDE_LBA_3 6
#define IDE_STATUS 7
#define IDE_COMMAND 7

#define IDE_DEVICE_0 0
#define IDE_DEVICE_1 0x10

/*Status*/
#define BSY (1<<7)
#define DRDY (1<<6)
#define DF (1<<5)
#define DSC (1<<4)
#define DRQ (1<<3)
#define CORR (1<<2)
#define IDX (1<<1)
#define ERR 1

typedef char BYTE;
typedef unsigned char UBYTE;
typedef int WORD;
typedef unsigned int UWORD;
typedef long DWORD;
typedef unsigned long UDWORD;

unsigned long spin_lock_1s = 0;

typedef struct _drive_info_struct {
    char model [41];
    char serial [21];
    char firmware_ver[9];
    char isATA;
    char isLBASupported;
    char isDMASupported;
    char isTRIMSupported;
    UWORD baseAddress;
    UBYTE device;
} drive_info_struct_t;


/* Gets the EFLAGS register. Mostly useful for checking if the program
 * is being run in Virtual 8086 mode ie. inside Windows
 */
unsigned long get_eflags(){
    register unsigned int high = 0, low = 0;
    unsigned long temp = 0;
    unsigned long ret = 0;
    asm .386;
    asm pushfd;
    asm pop low;
    asm pop high;
    temp = high;
    temp = temp << 16;
    ret = temp | low;
    return ret;
}

/*Returns a long representing approximately how many iterations of a loop would be required
 * To spin for one second. This is a very inaccurate method of estimation, but should work well
 * enough for the purposes of setting timeouts for hardware.
 */
unsigned long spin_lock_calibrate(){
    unsigned long i = 0;
    long timel1 = 0, timel2 = 0;
    time(&timel1);
    time(&timel2);
    while (timel2 - timel1 < 4){
        i++;
        if (i % 1000 == 0)
        time(&timel2);
    }
    return i>>2;
}

/*Gets the number of fixed disks in the system*/
int get_drive_count(){
    /*PC BIOS memory map*/
    /*http://fd.lod.bz/rbil/memory/bios/m00400075.html*/
    BYTE far * num_disks = 0;
    num_disks = MK_FP(0x40, 0x75);    
    return *num_disks;
}

/* trims trailing whitespace from a string */
void ltrim(char * str){
    int len = 0;
    int i = 0;
    len = strlen(str);
    for (i = len-1; i >= 0; i--){
        if (str[i] == 0x20){
            str[i] = 0;
        }else{
            break;
        }
    }
}

int get_drive_identity(WORD base, int drive_sel, UWORD * buffer_512b){
    int i = 0;
    long timeout = 0;
    memset(buffer_512b, 0, 512);
    /*Wait until the drive isn't busy*/
    while(get_ide_status(base) & BSY){
        timeout++;
        if (timeout > spin_lock_1s >> 1){return 0;}
    }
    /*disable interrupts*/
    disable();
    /*Select which IDE device to query*/
    outportb(base + IDE_LBA_3, drive_sel); /*0 selects dev 0, 0x10 selects device 1*/
    /*Write the command to the register - this sends the command to the drive*/
    outportb(base + IDE_COMMAND, 0xEC); /*IDENTIFY_DEVICE*/
    /*Wait for data*/
    timeout = 0;
    while(get_ide_status(base) & DRQ == 0){
        timeout++;
        if (timeout > spin_lock_1s >> 1){
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


int get_drive_info(drive_info_struct_t * di, WORD base, int drive_sel){
    int i = 0, j = 0;
    UWORD check = 0;
    UWORD buffer_512b[256];
    memset(di, 0, sizeof(drive_info_struct_t));
    memset(buffer_512b, 0, sizeof(buffer_512b));
    
    if(get_drive_identity(base, drive_sel, buffer_512b) == 0){
        return 0;
    }

    for(i=27; i<46; i++){
        di->model[j] = (buffer_512b[i]>>8);
        di->model[++j] = (buffer_512b[i]&0xFF);
        j++;
    }   
    j = 0;
    for(i=10; i<19; i++){
        di->serial[j] = (buffer_512b[i]>>8);
        di->serial[++j] = (buffer_512b[i]&0xFF);
        j++;
    }   
    j = 0;
    for(i=23; i<27; i++){
        di->firmware_ver[j] = (buffer_512b[i]>>8);
        di->firmware_ver[++j] = (buffer_512b[i]&0xFF);
        j++;
    }   
    ltrim(di->model);
    ltrim(di->serial);
    ltrim(di->firmware_ver);
    di->isATA = ((buffer_512b[0] & (1<<15)) >> 15) ^ 1;
    di->isDMASupported = (buffer_512b[49] & (1<<8)) >> 8;
    di->isLBASupported = (buffer_512b[49] & (1<<9)) >> 9;
    di->isTRIMSupported = buffer_512b[169] & 1;
    di->baseAddress = base;
    di->device = drive_sel;
    
    /*printf("BASE: %04X, CHECK: %04X\n", base, check >> 8);*/
    /* Sanity check. This value is always 0x80 as per ATA specs.*/
    check = buffer_512b[47];
    if(di->isATA && check >> 8 == 0x80){
        return 1;
    }else{
        return 0;
    }
    /* Words 
     * 64 -70 valid (buffer_512b[53] & 1)
     * 88 valid (buffer_512b[53] & 2)
     */
}

int get_ide_status(int base){
    return inportb(base + IDE_STATUS);
}

void print_drive_info(drive_info_struct_t * di){
    printf("%s %s FW: %s\n", di->model, di->serial, di->firmware_ver);
    printf("Base address: 0x%04X, device %d\n", di->baseAddress, di->device == 0 ? 0 : 1);
    printf("ATA: %d DMA: %d, TRIM: %d\n", di->isATA, di->isDMASupported, di->isTRIMSupported);
}

/*Returns 1 if being run in Virtual 8086 mode*/
int isVM86(){
    if (get_eflags() & (((long)1)<<17)){
        return 1;
    }
    return 0;
}

void detect_drives(){
    printf("Calibrating timers. This will take a few seconds.\n");
    spin_lock_1s = spin_lock_calibrate();
    printf("spin_lock_1s calibrated to %lu\n", spin_lock_1s);
    printf("Detecting drives...\n");
    if (get_ide_status(IDE_PRIMARY_1) != 0xFF){
        printf("Probing IDE PRIMARY...\n");
        /*at least one drive is connected*/
        if (get_drive_info(&(di[detected_drive_count]), IDE_PRIMARY_1, IDE_DEVICE_0)){
            detected_drive_count++;
        }
        if (get_drive_info(&(di[detected_drive_count]), IDE_PRIMARY_1, IDE_DEVICE_1)){
            detected_drive_count++;
        }
    }if (get_ide_status(IDE_SECONDARY_1) != 0xFF){
        printf("Probing IDE SECONDARY...\n");
        if (get_drive_info(&(di[detected_drive_count]), IDE_SECONDARY_1, IDE_DEVICE_0)){
            detected_drive_count++;
        }
        if (get_drive_info(&(di[detected_drive_count]), IDE_SECONDARY_1, IDE_DEVICE_1)){
            detected_drive_count++;
        }
    }
    
    if (detected_drive_count>0){
        printf("\n");
        for (i = 0; i < detected_drive_count; i++){
            print_drive_info(&(di[i]));
        }
    }else{
        printf("No drives detected.");
    }
}


int main (int argc, char * argv[]){
    int i = 0;
    int detected_drive_count  = 0;
    drive_info_struct_t di[4];
    memset(di, 0, sizeof(di));
    
    if(isVM86()){
        printf("This program cannot be run in virtual 8086 mode.\n");
        printf("(That means you need to be in DOS mode ;P)\n");
        return 0;
    }

    detect_drives();
    
    return 0;
}
