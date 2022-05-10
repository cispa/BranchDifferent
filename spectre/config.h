#ifndef CONFIG_H
#define CONFIG_H

/*
 * constants
 */

#define EVICTION 0
#define FLUSHING 1

#define MSR 0
#define COUNTER_THREAD 1


/*
 * generic configuration
 */
 
// higher value = more output
#define VERBOSITY 0

// use eviction
#define CACHE EVICTION

// use counter thread
#define TIMER COUNTER_THREAD

// page size
#define PAGE_SIZE 16384


/*
 * eviction configuration
 */
     
#if CACHE == EVICTION

    // iPhone 7: 150
    // iPhone 8 Plus: 120
    // M1 Mac mini: 220
    // threshold for finding eviction sets
    // if commented out, eviction set algorithm will calibrate it automatically
    // #define EVICTION_THRESHOLD 150

    // iPhone 7: 16
    // iPhone 8 Plus: 16
    // M1 Mac mini: 32
    // eviction set size
    #define EVICTION_SET_SIZE 16

    // size of memory to alloc
    #define EVICTION_MEMORY_SIZE 20 * 1024 * 1024

#endif /* EVICTION */


/*
 * spectre configuration
 */
 
// spectre variant (1 or 2)
#define VARIANT 1

// iPhone 7: 4
// iPhone 8 Plus: 4
// M1 Mac mini: 2
// bits to leak per victim call (1, 2, 4 or 8)
#define BITS 4

// entry size in array2
#define ENTRY_SIZE 512

// amount of measurement steps per leaked index.
#define ITERATIONS 4

// amount of calls to the victim function per measuerement step (ITERATIONS)
#define VICTIM_CALLS 40

// amount of training calls per out-of-bound call to the victim function.
// VICTIM_CALLS should be divisible by TRAINING + 1
#define TRAINING 9

// iPhone 7: 110
// iPhone 8 Plus: 60
// M1 Mac mini: 120
// threshold to distinguish cache miss from cache hit.
// (this is not used by the eviction code)
#define THRESHOLD 110

// Set to 1 to run a benchmark
#define BENCHMARK 1

/* --- calculated automatically --- */

// amount of offsets per byte
#define OFFSETS_PER_BYTE (8 / BITS)

// amount of values in array2
#define VALUES (1 << BITS)


/*
 * Mitigation
 */

// mitigation to insert
// uncomment to activate
// #define MITIGATION_ASM "HINT 0x14" /* CSDB */

#ifdef MITIGATION_ASM
    #define MITIGATION asm volatile(MITIGATION_ASM);
#else
    #define MITIGATION_ASM
    #define MITIGATION
#endif /* MITIGATION_ASM */

#endif /* CONFIG */
