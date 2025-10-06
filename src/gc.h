#ifndef GC_H
#define GC_H

#include <stddef.h>

#define GC_ALLOC_MAX 256

enum {
    GC_ALLOC,
    GC_REALLOC,
    GC_FREE,
    GC_DONE
};

typedef struct {
    size_t usage;
    size_t max_usage;
    unsigned long int allocation_sizes[GC_ALLOC_MAX];
    unsigned long int allocation_pos[GC_ALLOC_MAX];
    size_t allocations;
    size_t allocation_max;
    unsigned char allow_free;
    unsigned char allow_realloc;
    size_t taskpos;
    unsigned char task;

    unsigned long int old_size;

    unsigned long int seed;
} GC;

unsigned long int xorshift(unsigned long int *seed);

int gc_init(GC *gc, size_t alloc_max, unsigned char allow_free,
            unsigned char max_usage, unsigned char allow_realloc);
int gc_find_task(GC *gc, size_t *s, size_t *old_s);
void gc_task_done(GC *gc, unsigned long int pos);
void gc_task_fail(GC *gc);
void gc_free(GC *gc);

#endif
