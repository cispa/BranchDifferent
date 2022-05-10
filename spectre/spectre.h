#ifndef SPECTRE_H
#define SPECTRE_H

#include <stdint.h>

int leakByte(size_t index);

int leakValue(size_t offset);

int setup(char* public_data, char* secret_data);

#endif /* SPECTRE_H */
