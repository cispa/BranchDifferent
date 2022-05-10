# Spectre Proof of Concept #

Code for Spectre V1 and V2 PoC.


## configuring ##

Configuration can be done by adjusting macros in [`config.h`](config.h).

**Generic configuration**
* `VERBOSITY`: Amount of debug output. Higher value means more output.
* `CACHE`: Cache maintenance algorithm to use (`EVICTION` or `FLUSHING`)
* `TIMER`: Timer implementation to use (`COUNTER_THREAD` or `MSR`)

**Eviction configuration** (ignored if `CACHE` is not set to `EVICTION`)
* `EVICTION_THRESHOLD`: Threshold used to find eviction set. If removed, the algorithm will automatically calculate a threshold.
* `EVICTION_SET_SIZE`: Amount of addresses in an eviction set.

**Spectre configuration**
* `VARIANT`: Spectre variant to use (`1` or `2`)
* `BITS`: Amount of bits the gadget should leak (`2`, `4`, or `8`)
* `ENTRY_SIZE`: Size of each entry in public array
* `ITERATIONS`: Measurements per leaked value
* `VICTIM_CALLS`: Amount of total calls to the gadget per measurement
* `TRAINING`: Amount of training calls preceeding a misprediction (`VICTIM_CALLS` should be divisible by `TRAINING + 1`)
* `THRESHOLD`: Threshold to distinguish cache hits from misses
* `BENCHMARK`: Run the PoC in benchmark mode (`1` or `0`)
* `MITIGATION_ASM`: Assembly code that will be inserted (before out-of-bounds access or indirect jump) as mitigation

## compilation ##

**iPhone:** `make`
**M1 Mac:** `make m1`

## requirements ##
* `make`
* `clang`
* iphones only: [`ldid`](https://github.com/ProcursusTeam/ldid)
* iPhones only: SDK installed at `/var/sdk` (or change path in `Makefile`)


## run ##

`./spectre`

(after successful compilation the executable `spectre` should be there.)

## Output ##

### iPhone 7 ###

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
// #define EVICTION_THRESHOLD 150 /* use calibration! */
#define EVICTION_SET_SIZE 16
#define EVICTION_MEMORY_SIZE 20 * 1024 * 1024

/* --- Spectre --- */
#define VARIANT /* 1 or 2 */
#define BITS 4
#define ENTRY_SIZE 512
#define ITERATIONS 4
#define VICTIM_CALLS 40
#define TRAINING 9
#define THRESHOLD 110
#define BENCHMARK 1

/* --- Calculated automatically --- */
#define OFFSETS_PER_BYTE (8 / BITS)
#define VALUES (1 << BITS)

/* --- Mitigation --- */
#define MITIGATION_ASM
#define MITIGATION

#endif
```

**Variant 1:**
```
$ ./spectre
[Spectre Variant 1]
miss: 184, hit: 111, threshold: 166
Eviction set found!
Eviction set found!
finding eviction set failed!
finding eviction set failed!
Eviction set found!
leaked 10240 byte in 4923 ms. (2080.03 bytes/s) correct: 81438 / 81920 bits (99.41 %)
setup took: 770 ms.
```

**Variant 2:**
```
$ ./spectre
[Spectre Variant 2]
miss: 203, hit: 110, threshold: 180
finding eviction set failed!
finding eviction set failed!
Eviction set found!
Eviction set found!
Eviction set found!
leaked 10240 byte in 5055 ms. (2025.72 bytes/s) correct: 80573 / 81920 bits (98.36 %)
setup took: 816 ms.
```

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
// #define EVICTION_THRESHOLD 150 /* use calibration! */
#define EVICTION_SET_SIZE 16
#define EVICTION_MEMORY_SIZE 20 * 1024 * 1024

/* --- Spectre --- */
#define VARIANT /* 1 or 2 */
#define BITS 4
#define ENTRY_SIZE 512
#define ITERATIONS 4
#define VICTIM_CALLS 40
#define TRAINING 9
#define THRESHOLD 60
#define BENCHMARK 1

/* --- Calculated automatically --- */
#define OFFSETS_PER_BYTE (8 / BITS)
#define VALUES (1 << BITS)

/* --- Mitigation --- */
#define MITIGATION_ASM
#define MITIGATION

#endif
```

**Variant 1:**
```
$ ./spectre
[Spectre Variant 1]
miss: 160, hit: 44, threshold: 131
Eviction set found!
Eviction set found!
leaked 10240 byte in 6116 ms. (1674.30 bytes/s) correct: 80936 / 81920 bits (98.80 %)
setup took: 628 ms.
```

**Variant 2**
```
$ ./spectre
[Spectre Variant 2]
miss: 164, hit: 44, threshold: 134
Eviction set found!
Eviction set found!
leaked 10240 byte in 8829 ms. (1159.81 bytes/s) correct: 81267 / 81920 bits (99.20 %)
setup took: 667 ms.
```


### M1 Mac Mini ###

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
// #define EVICTION_THRESHOLD 220 /* use calibration */
#define EVICTION_SET_SIZE 32
#define EVICTION_MEMORY_SIZE 20 * 1024 * 1024

/* --- Spectre --- */
#define VARIANT /* 1 or 2 */
#define BITS 2
#define ENTRY_SIZE 512
#define ITERATIONS 4
#define VICTIM_CALLS 40
#define TRAINING 9
#define THRESHOLD 120
#define BENCHMARK 1

/* --- Calculated automatically --- */
#define OFFSETS_PER_BYTE (8 / BITS)
#define VALUES (1 << BITS)

/* --- Mitigation --- */
#define MITIGATION_ASM
#define MITIGATION

#endif
```

**Variant 1:**
```
% ./spectre
[Spectre Variant 1]
miss: 152, hit: 105, threshold: 140
finding eviction set failed!
Eviction set found!
Eviction set found!
leaked 10240 byte in 4403 ms. (2325.69 bytes/s) correct: 78632 / 81920 bits (95.99 %)
setup took: 302 ms.
```

**Variant 2:**
```
% ./spectre
[Spectre Variant 2]
miss: 167, hit: 101, threshold: 150
Eviction set found!
Eviction set found!
leaked 10240 byte in 5170 ms. (1980.66 bytes/s) correct: 81502 / 81920 bits (99.49 %)
setup took: 212 ms.
```

