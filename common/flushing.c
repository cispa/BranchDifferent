#include "config.h"

#if CACHE == FLUSHING

#include "cache.h"

cache_ctx_t cache_remove_prepare(char* address){
    return address;
}

void cache_remove(cache_ctx_t ctx){
    __asm__ volatile("DC CIVAC, %[address]" :: [address] "r" (ctx) : "memory");
}

#endif /* CACHE */
