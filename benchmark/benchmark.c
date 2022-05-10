
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#include "config.h"
#include "memory.h"
#include "timing.h"
#include "cache.h"

FILE* out;

static uint64_t __cache_miss(){
    uint64_t result = 0;
    
    char* buf = malloc(PAGE_SIZE * 100);
    
    int page = rand() % 99;
    
    // get translation into TLB
    memory_access(buf + page * PAGE_SIZE + (rand() % (PAGE_SIZE / 2)));
    
    result = probe(buf + page * PAGE_SIZE + PAGE_SIZE / 2 + (rand() % PAGE_SIZE / 2));
    
    free(buf);
    
    return result;
}


static uint64_t __cache_hit(){
    static char* buf = NULL;
    
    if(!buf){
        /* memory leak */
        buf = malloc(PAGE_SIZE);
    }  
    
    memory_access(buf + 1024);
    memory_access(buf + 1024);
    return probe(buf + 1024);
}

uint64_t __cache_maintaned(){
    static char* buf = NULL;
    static cache_ctx_t ctx;
    
    if(!buf){
        /* memory leak */
        buf = malloc(PAGE_SIZE);
        ctx = cache_remove_prepare(buf);
    }  

    memory_access(buf);
    memory_access(buf);
    memory_fence();
    cache_remove(ctx);
    return probe(buf);
}


void benchmark(uint64_t (*func)(void), uint64_t* results, char* header){
    for(int i = 0; i < SAMPLES; i++){
        results[i] = func();
    }
    
    fputs(header, out);
    for(int i = 0; i < SAMPLES; i++){
        fprintf(out, "%llu\n", results[i]);
    }
}

int main(int argc, char** argv){
    timer_start();
    
    // output file
    out = fopen(OUTPUT, "w");
    
    // result array
    uint64_t* results = malloc(sizeof(uint64_t) * SAMPLES);
    
    // cache hits
    benchmark(__cache_hit, results, "--- Cache hit ---");
    
    // cache misses
    benchmark(__cache_miss, results, "--- Cache miss ---");
    
    // maintaned cache
    benchmark(__cache_maintaned, results, "--- Cache maintained ---");
    
    timer_stop();
    
    free(results);
    
    return 0;
}
