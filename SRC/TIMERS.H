#ifndef __timers_h__
#define __timers_h__

#include "BRLNDTYP.H"
#include <DOS.H>

/*
 * MS-DOS Timer library that hooks INT 1C
 * See http://fd.lod.bz/rbil/interrup/bios/1c.html
 * 
 */

void far * __original_1C_vect = 0;
volatile uint32_t __timer_tick_count = 0;

uint32_t timers_get_tick_count();
void timers_delay_mills(uint32_t);
interrupt _timer_tick_int_handler();
void timers_initialize_timers();
void timers_unhook_timers();

#endif
