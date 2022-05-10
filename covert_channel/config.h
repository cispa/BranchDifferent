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
    // if removed, eviction set algorithm will calibrate
    #define EVICTION_THRESHOLD 150

    // iPhone 7: 16
    // iPhone 8 Plus: 16
    // M1 Mac mini: 32
    // eviction set size
    #define EVICTION_SET_SIZE 16

    // size of memory to alloc
    #define EVICTION_MEMORY_SIZE 20 * 1024 * 1024

#endif /* EVICTION */


/*
 * covert channel configuration
 */

// use optimized sender that removes cache lines after sending a bit
#define OPTIMIZED_SENDER 1

// time to send one bit (timer is running at 24 MHz on tested devices)
#define TRANSMISSION_TIME 10000

// offset of cache line accessed when transmitting 0 in mapped file
#define LINE_0_OFFSET ((4096 << 2)     + 256)

// offset of cache line accessed when transmitting 1 in mapped file
#define LINE_1_OFFSET ((4096 << 2) * 2 + 256)

// file to map (must fit both cache lines)
#define MAPPED_FILE_NAME "shared"

// receive threshold
// (this is not used by the eviction algorithm)
#define RECEIVE_THRESHOLD 50

// log timings in receiver instead of receiving characters
#define RECEIVER_LOG 1

// amount of timings to log (only if RECEIVER_LOG is 1)
#define LOG_AMOUNT 200000

// log output file
#define LOG_FILE "out.csv"

#endif /* CONFIG_H */
