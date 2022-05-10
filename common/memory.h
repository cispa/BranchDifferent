#ifndef MEMORY_H
#define MEMORY_H

// memory barrier
#define memory_fence() __asm__ volatile("DMB SY\nISB SY" ::: "memory")

// memory load
#define memory_access(x) __asm__ volatile("LDR x10, [%[addr]]" :: [addr] "r" (x) : "x10", "memory")

#endif /* MEMORY_H */
