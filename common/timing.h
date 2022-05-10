#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

#include "config.h"
#include "memory.h"

#if TIMER == COUNTER_THREAD
    extern uint64_t timestamp;
    #define timer_read(x) x = timestamp
#elif TIMER == MSR
    #define timer_read(x) __asm__ volatile("MRS %[target], CNTPCT_EL0" : [target] "=r" (x) :: "memory" );
#else 
    #error TIMER is set to an invalid value!
#endif /* TIMER */


static inline __attribute__((always_inline)) uint64_t probe(char* address){
    register uint64_t start, end;
    memory_fence();
    timer_read(start);
    memory_fence();
    memory_access(address);
    memory_fence();
    timer_read(end);
    memory_fence();
    return end - start;
}

void timer_start();

void timer_stop();

#endif /* TIMING_H */
