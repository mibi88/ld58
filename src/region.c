#include <region.h>
#include <gfx.h>
#include <boundingbox.h>

#include <stdio.h>
#include <string.h>

#define ASSETS (region->assets)

int region_init(Region *region, Assets *assets, size_t size,
                int preview_alloc_size) {
    ASSETS = assets;

    region_reset(region, size, preview_alloc_size);

    return 0;
}

void region_reset(Region *region, size_t size, int preview_alloc_size) {
    region->size = size;
    region->preview = preview_alloc_size;
    memset(region->region, 0, size);

    region->selected = -1;
}

int region_draw(Region *region, int x, int y, int w, size_t alloc_len) {
    size_t i;
    char buffer[16];

    int tile_size = ASSETS->tile_size*ASSETS->scale;
    int sx = x;

    int block_hovered = 0;
    int invalid = 0;

    size_t first;

    y += tile_size/2;

    for(i=0;i<region->size;i++){
        int tile = region->region[i]&REGION_BLOCK_ALLOCATED ? 1 : 0;

        gfx_subimage(&ASSETS->tiles, x, y, tile_size, tile_size,
                     ASSETS->tile_size*tile, 0, ASSETS->tile_size,
                     ASSETS->tile_size, 0, 255, 255, 255);

        if(BB_IN(gfx_mouse_x(), gfx_mouse_y(), x-tile_size/2,
                 y-tile_size/2, tile_size, tile_size)){
            block_hovered = 1;
            first = i;
            if(region->size-i < alloc_len) invalid = 1;
        }
        if(region->preview){
            if(alloc_len > 0 && block_hovered){
                gfx_subimage(&ASSETS->tiles, x, y, tile_size, tile_size,
                             invalid*ASSETS->tile_size, ASSETS->tile_size,
                             ASSETS->tile_size, ASSETS->tile_size, 0, 255,
                             255, 255);
                alloc_len--;
            }
        }

        if(region->region[i]&REGION_BLOCK_ALLOC_START){
            sprintf(buffer, "%lu", (unsigned long int)i);
            gfx_text(&ASSETS->font, x+REGION_TILE_PADDING*ASSETS->scale, y,
                     buffer, 255, 255, 255, ASSETS->scale);
        }

        x += tile_size;
        if(x > sx+w && i != region->size-1){
            x = sx;
            y += tile_size;
        }
    }

    if(block_hovered && !invalid){
        region->selected = first;
    }else{
        region->selected = -1;
    }

    return y+tile_size;
}

int region_alloc(Region *region, size_t alloc_len,
                 long unsigned int *corruption) {
    size_t i;

    if(region->selected < 0) return 1;

    region->region[region->selected] |= REGION_BLOCK_ALLOC_START;

    for(i=region->selected;i<region->selected+alloc_len;i++){
        if(region->region[i]&REGION_ALLOC_INTERNAL) (*corruption)++;
        region->region[i] |= REGION_BLOCK_ALLOCATED|REGION_ALLOC_INTERNAL;
    }

    region->selected = -1;

    return 0;
}

void region_free_alloc(Region *region, size_t pos, size_t alloc_len) {
    size_t i;

    for(i=pos;i<pos+alloc_len;i++){
        if((region->region[i]&REGION_BLOCK_ALLOC_START) && i != pos) break;
        region->region[i] &= REGION_ALLOC_INTERNAL;
    }

    pos = -1;
}

void region_free_internal(Region *region, size_t pos, size_t alloc_len) {
    size_t i;

    for(i=pos;i<pos+alloc_len;i++){
        region->region[i] &= ~REGION_ALLOC_INTERNAL;
    }
}

void region_free(Region *region) {
    region->size = 0;
}
