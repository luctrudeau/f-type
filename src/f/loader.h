#ifndef f_loader_h
#define f_loader_h

#include "../common.h"

#define HEADER_SIZE 0x10

typedef struct Driver Driver;

int ines_loader(Driver *driver, uint8_t *rom_data, int rom_data_size);

void f_teardown(Driver *driver);

#endif /* f_loader_h */
