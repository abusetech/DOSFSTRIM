#ifndef __timers_h__
#define __timers_h__

#include "BRLNDTYP.H"
#include <DOS.H>

/*
 * MS-DOS Timer library that hooks INT 1C
 * Based on information from http://fd.lod.bz/rbil/interrup/bios/1c.html
 * 
 */

#define TIMERS_MILLISECONDS_PER_TICK 55

/* void	 _Cdecl setvect	(int interruptno, void interrupt (*isr) ()); */

void interrupt (* __original_1C_vect)() = 0;
volatile uint32_t __timer_tick_count = 0;
uint32_t far * __timer_tick_ptr = 0;

uint32_t timers_get_tick_count();
void timers_delay_millis(uint32_t);
void interrupt far _timer_tick_int_handler();
void timers_initialize_timers();
void timers_unhook_timers();
uint8_t timers_wait_until_IO_bit_set_timeout(uint16_t addr, uint8_t mask, uint32_t timout_ms);
uint8_t timers_wait_until_IO_bit_clear_timeout(uint16_t addr, uint8_t mask, uint32_t timout_ms);
#endif
