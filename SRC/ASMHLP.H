#include "BRLNDTYP.H"
#include <DOS.H>

#ifndef __ASMHLP_H__
#define __ASMHLP_H__

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
 
*/

extern void far cdecl outportl32(uint16_t addr, uint32_t dword);

/*
 
 outportl32() write 32bit word to I/O port
 void far cdecl outportl32(uint16_t addr, uint32_t dword);
 
*/

extern uint32_t far cdecl inportl32(uint16_t addr);

#endif
