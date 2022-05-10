# Covert Channel #

Code for Covert Channel.


## configuring ##

Configuration can be done by adjusting macros in [`config.h`](config.h).

**Generic configuration**
* `VERBOSITY`: Amount of debug output. Higher value means more output.
* `CACHE`: Cache maintenance algorithm to use (`EVICTION` or `FLUSHING`)
* `TIMER`: Timer implementation to use (`COUNTER_THREAD` or `MSR`)

**Eviction configuration** (ignored if `CACHE` is not set to `EVICTION`)
* `EVICTION_THRESHOLD`: Threshold used to find eviction set. If removed, the algorithm will automatically calculate a threshold.
* `EVICTION_SET_SIZE`: Amount of addresses in an eviction set.

**Covert channel configuration**
* `OPTIMIZED_SENDER`: Enable optimized sender which will evict cache lines from its private cache after transmitting a bit (`0` or `1`)
* `TRANSMISSION_TIME`: Time to transmit a single bit (timer was running at 24MHz on tested devices)
* `MAPPED_FILE_NAME`: Name of shared file to map
* `RECEIVE_THRESHOLD`: Threshold for distinguishing cache hits from cache misses
* `RECEIVER_LOG`: Run receiver of covert channel in log mode, e.g. log all timings to a file (`0` or `1`)
* `LOG_AMOUNT`: If receiver is running in log mode, how many timings should be recorded for both cache lines
* `LOG_FILE`: If receiver is running in log mode, name of file to write results to.

## compilation ##

**iPhone:** `make`
**M1 Mac:** `make m1`

## requirements ##
* `make`
* `clang`
* iphones only: [`ldid`](https://github.com/ProcursusTeam/ldid)
* iPhones only: SDK installed at `/var/sdk` (or change path in `Makefile`)


## run ##

**sender:** `./sender`
**receiver:** `./receiver`

(after successful compilation the executables `sender` and `receiver` should be there.)

## Output ##

As described in the paper, the covert channel does not work on the iPhone 7.

### iPhone 8 Plus ###

**config.h:**
```
#ifndef CONFIG_H
#define CONFIG_H

/* --- Constants --- */
#define EVICTION 0
#define FLUSHING 1
#define MSR 0
#define COUNTER_THREAD 1

/* --- Generic --- */
#define VERBOSITY 0
#define CACHE EVICTION
#define TIMER COUNTER_THREAD
#define PAGE_SIZE 16384

/* --- Eviction --- */
// #define EVICTION_THRESHOLD 120 /* calibrate */
#define EVICTION_SET_SIZE 16
#define EVICTION_MEMORY_SIZE 20 * 1024 * 1024

/* --- Covert Channel --- */
#define OPTIMIZED_SENDER 1
#define TRANSMISSION_TIME 100000
#define LINE_0_OFFSET ((4096 << 2)     + 256)
#define LINE_1_OFFSET ((4096 << 2) * 2 + 256)
#define MAPPED_FILE_NAME "shared"
#define RECEIVE_THRESHOLD 60
#define RECEIVER_LOG 0 /* disable logging */
#define LOG_AMOUNT 200000
#define LOG_FILE "out.csv"
#endif /* CONFIG_H */
```

**sender:**
```
$ ./sender
miss: 135, hit: 44, threshold: 113
Eviction set found!
Eviction set found!
Press (almost) any key to start transmission

Sending . . .
```

**receiver:**
```
$ ./receiver
miss: 156, hit: 44, threshold: 128
Eviction set found!
Eviction set found!
B��
-+0Hello World
Hello World
Hello World
Hello World
Hello World
Hello World
Hello World
```


### M1 Mac mini ###

**config.h:**
```
#ifndef CONFIG_H
#define CONFIG_H

/* --- Constants --- */
#define EVICTION 0
#define FLUSHING 1
#define MSR 0
#define COUNTER_THREAD 1

/* --- Generic --- */
#define VERBOSITY 0
#define CACHE EVICTION
#define TIMER COUNTER_THREAD
#define PAGE_SIZE 16384

/* --- Eviction --- */
#define EVICTION_THRESHOLD 220
#define EVICTION_SET_SIZE 32
#define EVICTION_MEMORY_SIZE 20 * 1024 * 1024

/* --- Covert Channel --- */
#define OPTIMIZED_SENDER 1
#define TRANSMISSION_TIME 100000
#define LINE_0_OFFSET ((4096 << 2)     + 256)
#define LINE_1_OFFSET ((4096 << 2) * 2 + 256)
#define MAPPED_FILE_NAME "shared"
#define RECEIVE_THRESHOLD 130
#define RECEIVER_LOG 0 /* disable logging */
#define LOG_AMOUNT 200000
#define LOG_FILE "out.csv"
#endif /* CONFIG_H */
```

**sender:**
```
% ./sender
Eviction set found!
Press (almost) any key to start transmission

Sending . . .
```

**receiver:**
```
% ./receiver
Eviction set found!
�Hello World
Hello World
Hello World
Hello World
Hello World
Hello World
Q�
```

