#include <gc.h>

#include <stdio.h>
#include <time.h>

#include <string.h>

unsigned long int xorshift(unsigned long int *seed) {
    /* See p.4 of "Xorshift RNGs" */

    *seed ^= *seed<<13;
    *seed ^= *seed>>17;
    *seed ^= *seed<<5;

    return *seed;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int gc_init(GC *gc, size_t alloc_max, unsigned char max_usage,
            unsigned char allow_free, unsigned char allow_realloc) {
    gc->usage = 0;
    gc->max_usage = max_usage;
    gc->allocations = 0;
    gc->allocation_max = alloc_max;
    gc->allow_free = allow_free;
    gc->allow_realloc = allow_realloc;

    gc->seed = clock();

    printf("seed: %lu\n", gc->seed);

    memset(gc->allocation_sizes, 0, sizeof(unsigned long int)*GC_ALLOC_MAX);
    memset(gc->allocation_pos, 0, sizeof(unsigned long int)*GC_ALLOC_MAX);

    return 0;
}

int gc_find_task(GC *gc, size_t *s, size_t *old_s) {
    size_t i;
    unsigned char task = xorshift(&gc->seed)&3;
    size_t idx;

    if(!gc->allocations || ((task == GC_ALLOC ||
                             (!gc->allow_realloc && !gc->allow_free)) &&
                            gc->allocations < gc->allocation_max &&
                            gc->usage < gc->max_usage)){
        for(i=0;i<gc->allocation_max;i++){
            if(!gc->allocation_sizes[i]){
                size_t size;

                size = xorshift(&gc->seed)%MAX(1,
                                        MIN(gc->max_usage/gc->allocation_max,
                                            gc->max_usage-gc->usage));
                size = MAX(1, size);

                gc->allocation_sizes[i] = size;
                gc->allocations++;
                *s = size;
                gc->usage += size;
                gc->taskpos = i;
                gc->task = GC_ALLOC;
                return GC_ALLOC;
            }
        }
    }else{
        if(!gc->allow_free && !gc->allow_realloc) return GC_DONE;
        task = (xorshift(&gc->seed)&1)+1;
    }
    if(!gc->allocations) return GC_DONE;
    if((task == GC_REALLOC || !gc->allow_free) && gc->usage < gc->max_usage &&
       gc->allow_realloc){
        size_t idx = xorshift(&gc->seed)&gc->allocation_max;
        size_t size;

        size = xorshift(&gc->seed)&MIN(5, gc->max_usage-gc->usage);
        size = MAX(1, size);

        for(i=idx;i!=gc->allocation_max;i=(i+1)%gc->allocation_max){
#if 0
            printf("%lu, %lu\n", i, gc->allocation_sizes[i]);
#endif
            if(gc->allocation_sizes[i]){
                *old_s = gc->allocation_sizes[i];
                gc->old_size = gc->allocation_sizes[i];

                gc->allocation_sizes[i] += size;
                *s = gc->allocation_sizes[i];
                gc->usage += size;
                gc->taskpos = i;

                gc->task = GC_REALLOC;
                return GC_REALLOC;
            }
        }
    }else{
        if(!(xorshift(&gc->seed)&63) && gc->allow_realloc) return GC_DONE;
    }

    /* Free stuff */

    idx = xorshift(&gc->seed)&gc->allocation_max;
    for(i=idx;i!=gc->allocation_max;i=(i+1)%gc->allocation_max){
#if 0
        printf("%lu, %lu\n", i, gc->allocation_sizes[i]);
#endif
        if(gc->allocation_sizes[i]){
            *s = gc->allocation_sizes[i];
            gc->old_size = gc->allocation_sizes[i];

            gc->usage -= gc->allocation_sizes[i];
            gc->allocations--;

            gc->allocation_sizes[i] = 0;

            gc->taskpos = i;
            gc->task = GC_FREE;
            return GC_FREE;
        }
    }

    return GC_DONE;
}

void gc_task_done(GC *gc, unsigned long int pos) {
    printf("Task at %lu done at %04lx\n", gc->taskpos, pos);

    gc->allocation_pos[gc->taskpos] = pos;
}

void gc_task_fail(GC *gc) {
    /* NOTE: One cannot fail frees */

    puts("Fail task");

    switch(gc->task){
        case GC_ALLOC:
            gc->allocations--;
            gc->usage -= gc->allocation_sizes[gc->taskpos];
            gc->allocation_sizes[gc->taskpos] = 0;
            break;
        case GC_REALLOC:
            gc->usage -= gc->allocation_sizes[gc->taskpos]-gc->old_size;
            gc->allocation_sizes[gc->taskpos] = gc->old_size;
            break;
    }

    /* To avoid issues if gc_task_fail is called more than once */
    gc->task = GC_DONE;
}

void gc_free(GC *gc) {
    (void)gc;
    /**/
}
