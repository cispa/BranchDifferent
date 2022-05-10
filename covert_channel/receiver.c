#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>

#include "config.h"
#include "cache.h"
#include "memory.h"
#include "timing.h"


// line accessed when sending 0
char * LINE_DATA_0;

// line accessed when sending 1
char * LINE_DATA_1;

// read only mapped file containing cache lines
char* mapped_file;

// context to remove LINE_DATA_0 from cache
cache_ctx_t LINE_DATA_0_CTX;

// context to remove LINE_DATA_1 from cache
cache_ctx_t LINE_DATA_1_CTX;


static inline void get_timestamp(uint64_t * value){
    __asm__ volatile(
        "MRS %[target], CNTPCT_EL0" /* read CNTPCT_EL0 model specific register */
        : [target] "=r" (*value) /* write result to *value */
        : /* no register read */
        : /* no additional register changed */
    );
}


char* map_file(char* file){
    int fd = open(file, O_RDONLY);
    if(fd == -1){
        return NULL;
    }
    struct stat file_info;
    if(fstat(fd, &file_info) == -1){
        return NULL;
    }
    
    if(file_info.st_size <= LINE_0_OFFSET ||
       file_info.st_size <= LINE_1_OFFSET){
        fprintf(stderr, "mapped file is too small!");
        return NULL;  
    }
    
    return mmap(0, file_info.st_size, PROT_READ, MAP_SHARED, fd, 0);
}

static inline int receive_bit(uint64_t end){
    uint64_t cur;
    int hits_0 = 0;
    int hits_1 = 0;
 
    do{
        hits_0 += probe(LINE_DATA_0) < RECEIVE_THRESHOLD;
        hits_1 += probe(LINE_DATA_1) < RECEIVE_THRESHOLD;
    	cache_remove(LINE_DATA_0_CTX);
    	cache_remove(LINE_DATA_1_CTX);
    } while(get_timestamp(&cur), cur < end);

    return hits_1 > hits_0;    
}

static inline int receive_byte(){
    uint64_t cur;
    
    do {
        get_timestamp(&cur);
    } while((cur / TRANSMISSION_TIME) % 8);
    
    cur -= cur % TRANSMISSION_TIME;
    
    int received = 0;
    for(int i = 1; i <= 8; i++){
        received <<= 1;
        received |= receive_bit(cur + i*TRANSMISSION_TIME);
    }
    
    return received;
}

#if RECEIVER_LOG == 1

typedef struct {
    uint64_t time;
    uint64_t line_0;
    uint64_t line_1;
} log_entry_t;

static inline log_entry_t* log(int amount){
    
    log_entry_t* results = malloc(sizeof(log_entry_t) * amount);

    for(int i = 0; i < amount; i++){
    
#if CACHE == FLUSHING
    for(int i = 0; i < 100; i++) __asm__ volatile("nop");
#endif /* CACHE */
    
        get_timestamp(&results[i].time);
        results[i].line_0 = probe(LINE_DATA_0);
        results[i].line_1 = probe(LINE_DATA_1);
        
#if CACHE == FLUSHING
    for(int i = 0; i < 100; i++) __asm__ volatile("nop");
#endif /* CACHE */
        
        cache_remove(LINE_DATA_0_CTX);
    	cache_remove(LINE_DATA_1_CTX);
    }
    
    return results;
}

#endif /* RECEIVER_LOG */


int main(int argc, char** argv){
    
    timer_start();
    
    mapped_file = map_file(MAPPED_FILE_NAME);
    LINE_DATA_0 = &mapped_file[LINE_0_OFFSET];
    LINE_DATA_1 = &mapped_file[LINE_1_OFFSET];
   
    LINE_DATA_0_CTX = cache_remove_prepare(LINE_DATA_0);
    LINE_DATA_1_CTX = cache_remove_prepare(LINE_DATA_1);
#if RECEIVER_LOG == 1
    
    puts("ready, press enter to start");
    getchar();
    puts("recording");

    log_entry_t* results = log(LOG_AMOUNT);
    
    uint64_t starttime = results[0].time;
    starttime -= starttime % TRANSMISSION_TIME;
    
    FILE* out = fopen(LOG_FILE, "w");
    fprintf(out, "bit, data 0, data 1\n");
    for(int i = 0; i < LOG_AMOUNT; i++){
        fprintf(out, "%f,%" PRId64 ",%" PRId64 "\n", 
               (double)(results[i].time - starttime) / TRANSMISSION_TIME, results[i].line_0, results[i].line_1);                   
    }
    fclose(out);
    
    free(results);
#else
    for(;;){
        putchar(receive_byte());
        fflush(stdout);
    }
#endif /* RECEIVER_LOG */
   
    timer_stop(); 
}
