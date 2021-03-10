#include "TIMERS.H"


uint32_t timers_get_tick_count(){
    return __timer_tick_count;
}

void timers_delay_mills(uint32_t delay){
    uint32_t start = __timer_tick_count;
    while((__timer_tick_count - start) / 55 < delay){}
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

