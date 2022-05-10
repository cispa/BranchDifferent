#include "config.h"

#if TIMER == COUNTER_THREAD

#include "timing.h"

#include <pthread.h>
#include <stdint.h>

uint64_t timestamp;

pthread_t counter_thread;

int timers_active = 0;

void* counter_thread_thread(void* ctx){

    __asm__ volatile(
        "LDR x10, [%[counter]]\n"       // load counter once
        "loop:\n"                       // while(true) {
            "ADD x10, x10, #1\n"        //     increment counter
            "STR x10, [%[counter]]\n"   //     store counter to memory
        "B loop\n"                      // }
        
        : /* no written registers */
        : [counter] "r" (ctx) /* read ctx (pointer to counter) */
        : "x10", "memory" /* writes x10 (holds counter value) */
    );

    return NULL;
}

void timer_start(){
    if(!(timers_active ++)){
        pthread_create (&counter_thread, NULL, counter_thread_thread, &timestamp);
    }
}

void timer_stop(){
    if(!(--timers_active)){
        pthread_cancel(counter_thread);
    }
}

#endif /* TIMER */
