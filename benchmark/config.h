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
 * benchmark configuration
 */
 
// how many data points to record
#define SAMPLES 1000000

// name of output file
#define OUTPUT "out.txt"

#endif /* CONFIG */
