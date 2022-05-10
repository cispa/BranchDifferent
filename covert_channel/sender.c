#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

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

#if OPTIMIZED_SENDER

// context to remove LINE_DATA_0 from cache
cache_ctx_t LINE_DATA_0_CTX;

// context to remove LINE_DATA_1 from cache
cache_ctx_t LINE_DATA_1_CTX;

#endif /* OPTIMIZED_SENDER */

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

static inline void send_bit(int value, uint64_t end){
    uint64_t cur;
    
    char* LINE_DATA = value ? LINE_DATA_1 : LINE_DATA_0;
    
    while(get_timestamp(&cur), cur < end){
        memory_access(LINE_DATA);
    }
    
#if OPTIMIZED_SENDER
    cache_remove(value ? LINE_DATA_1_CTX : LINE_DATA_0_CTX);
#endif /* OPTIMIZED_SENDER */
}

static inline void send_byte(char value){
    uint64_t cur;
    
    do {
        get_timestamp(&cur);
    } while((cur / TRANSMISSION_TIME) % 8);
    
    cur -= cur % TRANSMISSION_TIME;

    for(int i = 0; i < 8; i++){
        send_bit(value & (128 >> i), cur + (i+1) * TRANSMISSION_TIME);
    }
}

int main(int argc, char** argv){

    mapped_file = map_file(MAPPED_FILE_NAME);
    LINE_DATA_0 = &mapped_file[LINE_0_OFFSET];
    LINE_DATA_1 = &mapped_file[LINE_1_OFFSET];
    
#if OPTIMIZED_SENDER
    timer_start();
    LINE_DATA_0_CTX = cache_remove_prepare(LINE_DATA_0);
    LINE_DATA_1_CTX = cache_remove_prepare(LINE_DATA_1);
    timer_stop();
    
    puts("Press (almost) any key to start transmission");
    getchar();
    puts("Sending . . .");
#endif /* OPTIMIZED_SENDER */
    
    char* text = argc > 1 ? argv[1] : "Hello World\n";
    int len = strlen(text);
    
    for(;;){
        for(int i = 0; i < len; i++) send_byte(text[i]);
    }
}
