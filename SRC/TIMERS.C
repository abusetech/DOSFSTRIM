#include "TIMERS.H"


uint32_t timers_get_tick_count(){
    return __timer_tick_count;
}

void timers_delay_millis(uint32_t delay){
    uint32_t start = __timer_tick_count;
    while(((__timer_tick_count - start) /  TIMERS_MILLISECONDS_PER_TICK) < delay){}
}

interrupt _timer_tick_int_handler(){
    disable();
    __timer_tick_count++;
    enable();
}

void timers_initialize_timers(){
    __original_1C_vect = getvect(0x1C);
    __timer_tick_count = 0;
    disable();
    setvect(0x1C, _timer_tick_int_handler);
    enable();
}

void timers_unhook_timers(){
    disable();
    setvect(0x1C, __original_1C_vect);
    enable();
    __timer_tick_count = 0;
    __original_1C_vect = 0;
}

uint8_t wait_until_IO_bit_set_timeout(uint16_t addr, uint8_t mask, uint32_t timout_ms){
    /*
     * Loops until the bit(s) specified by mask are set in the 8 bit I/O port at addr
     * or until the timeout_ms milliseconds have elapsed.
     * 
     * If the timer expires, this function returns zero
     * If one or more bits are set, this function returns a value indicating which 
     * bits are set specifically, (mask & <io port value>)
     * 
     */
    uint8_t prt = 0;
    uint32_t start = 0;
    while(((__timer_tick_count - start) /  TIMERS_MILLISECONDS_PER_TICK) < timeout_ms){
        prt = inportb(addr) & mask;
        if (prt){
            return prt;
        }
    }
    return 0;
}

