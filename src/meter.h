#ifndef METER_H
#define METER_H

#include <stddef.h>

#define METER_START 2
#define METER_END 4
#define METER_MAX_LEN 128

typedef struct {
    char text[METER_MAX_LEN*2];
    size_t size;
} Meter;

int meter_init(Meter *meter, char *name, size_t size, size_t value);
void meter_set_value(Meter *meter, size_t value);
void meter_free(Meter *meter);

#endif
