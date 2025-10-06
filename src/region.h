#ifndef REGION_H
#define REGION_H

#include <stddef.h>
#include <assets.h>

#define REGION_MAX 64
#define REGION_BLOCK_ALLOCATED 0x80
#define REGION_BLOCK_ALLOC_START 0x40
#define REGION_ALLOC_INTERNAL 0x20
#define REGION_TILE_PADDING 2

typedef struct {
    size_t size;
    unsigned char preview;
    unsigned char region[REGION_MAX];
    long int selected;
    Assets *assets;
} Region;

int region_init(Region *region, Assets *assets, size_t size,
                int preview_alloc_size);
void region_reset(Region *region, size_t size, int preview_alloc_size);
int region_draw(Region *region, int x, int y, int w, size_t alloc_len);
int region_alloc(Region *region, size_t alloc_len,
                 long unsigned int *corruption);
void region_free_alloc(Region *region, size_t pos, size_t alloc_len);
void region_free_internal(Region *region, size_t pos, size_t alloc_len);
void region_free(Region *region);

#endif
