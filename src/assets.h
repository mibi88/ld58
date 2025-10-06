#ifndef ASSETS_H
#define ASSETS_H

#include <gfx.h>

#define ASSETS_HELP_LEN 7

typedef struct {
    GFXTexture font;
    GFXTexture tiles;
    GFXTexture menu;
    GFXTexture well_done;
    GFXTexture game_over;

    GFXTexture help_screens[ASSETS_HELP_LEN];

    int tile_size;
    float scale;
} Assets;

#endif
