#include <stdio.h>
#include "TIMERS.H"
#include "TIMERS.C"

main(){
    int i = 0;
    void far * pre_init_1c_vect = 0;
    void far * post_init_1c_vect = 0;
    
    printf("Testing timers\n");
    pre_init_1c_vect = getvect(0x1C);
    printf("Testing timers: initialization\n");
    timers_initialize_timers();
    post_init_1c_vect = getvect(0x1C);
    printf("Testing timers: old pointer at IVT #1C: %Fp\n", pre_init_1c_vect);
    printf("Testing timers: new pointer at IVT #1C: %Fp\n", post_init_1c_vect);
    printf("Counting to 5 seconds.\n");
    for (i=0; i < 5; i++){
        printf("%d ", i);
        timers_delay_millis(1000);
    }
    printf("\n");
    timers_unhook_timers();
    printf("Testing timers: pointer restored to IVT #1C: %Fp\n", getvect(0x1C));
}
