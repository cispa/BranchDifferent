# Benchmark #

Code for testing timers and cache maintanence.
Records timings of cache misses, cache hits, and memory accesses after cache maintenance.

## configuring ##

Configuration can be done by adjusting macros in [`config.h`](config.h).

**Generic configuration**
* `VERBOSITY`: Amount of debug output. Higher value means more output.
* `CACHE`: Cache maintenance algorithm to use (`EVICTION` or `FLUSHING`)
* `TIMER`: Timer implementation to use (`COUNTER_THREAD` or `MSR`)

**Eviction configuration** (ignored if `CACHE` is not set to `EVICTION`)
* `EVICTION_THRESHOLD`: Threshold used to find eviction set. If removed, the algorithm will automatically calculate a threshold.
* `EVICTION_SET_SIZE`: Amount of addresses in an eviction set.

**Benchmark configuration**
* `SAMPLES`: Amount of measurements to take
* `OUTPUT`: File to write results to

## compilation ##

**iPhone:** `make`
**M1 Mac:** `make m1`

## requirements ##
* `make`
* `clang`
* iphones only: [`ldid`](https://github.com/ProcursusTeam/ldid)
* iPhones only: SDK installed at `/var/sdk` (or change path in `Makefile`)
* conversion tool: `Python 3`

## run ##

`./benchmark`

to convert the output into a csv file: `python3 convert.py [filename]`.
