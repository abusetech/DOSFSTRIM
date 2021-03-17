#include "BRLNDTYP.H"

/*
 * 
 * ASMHLP - Assembly language helper functions
 * 
 * These are already compiled routines that can be executed to preform various functions
 * These functions are largely necessary because Borland Turbo C 2.0 only supports
 * the 80186 instruction set and some 32 bit instructions are needed for low level hardware I/O
 * 
 */

/*

 inportl32() read 32bit double word from I/O register using one 32bit read
 
;IN EAX, DX  Input doubleword from I/O port in DX into EAX

;uint32_t far cdecl inportl(uint16_t addr)

;low word returned in AX, high word returned in DX
;P. 266 of the Turbo C User's guide

push bp
mov bp, sp
mov dx,[bp+6]   ;move addr into DX (pushed to SP+6 before call) 
in eax, dx      ;read 32bit port at address DX into EAX
mov edx, eax    ;copy the result to edx
shr edx, 16     ;shift the bits in edx so that dx contains the upper word
pop bp          ;skip setting SP = BP as we didn't push anything else to the stack
ret             ;return

 {  } 
*/

uint8_t __inportl32_shellcode =  { 0x66, 0x55, 0x66, 0x89, 0xE5, 0x67, 0x66, 0x8B, 0x56, 0x06, 0xED, 0x89, 0xC2, 0xC1, 0xEA, 0x10, 0x66, 0x5D, 0xC3 };

uint32_t (far cdecl *inportl32)(uint16_t addr) = ((far cdecl*)(uint16_t))__inportl32_shellcode;

/*
 
 OUT portl
 
 void far cdecl outportl(uint16_t addr, uint32_t dword);
 
 push bp
 mov bp, sp
 mov dx, [bp+10]    ;addr
 mov eax, [bp+8]    ;dword
 out dx, eax        ;output word in eax to address in dx
 pop bp             ;restore bp
 ret
  
*/

uint8_t __outportl32_shellcode = { 0x66, 0x55, 0x66, 0x89, 0xE5, 0x67, 0x66, 0x8B, 0x56, 0x0A, 0x67, 0x8B, 0x46, 0x08, 0xEF, 0x66, 0x5D, 0xC3 } 

void (far cdecl *outportl32)(uint16_t addr, uint32_t dword) = __outportl32_shellcode;
