# Branch Different - Spectre attacks on Apple silicon #

<img src="logo.png" width="512" height="405">

Code for Spectre V1 and V2 PoC and covert channel used in the paper.


## Overview
Each folder contains a `Readme.md` with additional documentation.

### [common](common)
Timing and cache maintenance code required by both, the spctre PoC and the covert channel.
The eviction code is a strongly modified version of [evsets](https://github.com/cgvwzq/evsets).

### [covert_channel](covert_channel)
Code of the Covert Channel.

### [spectre](spectre)
Code of Spectre V1 and Spectre V2 PoC.

### [benchmark](benchmark)
Code used for benchmarking timing and eviction
