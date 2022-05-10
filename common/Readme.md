# Cache Maintenance and Timing #

Code for accurate timing and cache maintenance.


## Overview

### [cache.h](cache.h)
Header file for cache maintenance.
* `cache_ctx_t cache_remove_prepare(void* addr)`: Prepares a context that can be used with `cache_remove` to remove the cache line containing `addr` from the cache.
* `void cache_remove(cache_ctx_t ctx)`: Removes the cache line associated with `ctx` from the cache.

implemented by:
* `CACHE == EVICTION`: [`eviction.c`](eviction.c)
* `CACHE == FLUSHING`: [`flushing.c`](flushing.c)

### [memory.h](memory.h)
Header file for memory barrier and memory access.
* `void memory_fence()`: A full memory barrier (DMB SY + ISB SY).
* `void memory_access(void* addr)`: A memory access primitive.

### [timing.h](timing.h)
Header file for timing.
* `timer_read(time)`: Gets the current timer value. Usage: `uint64_t time; timer_read(time);`
* `uint64_t probe(void* addr)`: Time a memory load. The load is shielded with memory fences.
* `void timer_start()`: Prepare the timer. This must be called before `timer_read` or `probe`
* `void timer_stop()`: Stop the timer.

implemented by:
* `TIMER == COUNTER_THREAD`: [`counter_thread.c`](counter_thread.c)
* `TIMER == MSR`: [`msr.c`](msr.c)
