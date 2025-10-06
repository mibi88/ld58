#ifndef GAME_H
#define GAME_H

#include <assets.h>
#include <gfx.h>
#include <region.h>
#include <gc.h>
#include <meter.h>

#define GAME_LEVEL_AMOUNT 8
#define GAME_MAX_REGIONS 6

#define GAME_DIFFICULTY_Y (gfx_h()/2)

#define GAME_METER_SIZE 10

#define GAME_FIRST_LEVEL 0

#define GAME_TASK_ANIM_DURATION 0.25
#define GAME_TASK_ANIM_OFFSET 4

enum {
    GAME_TITLE,
    GAME_HELP,
    GAME_DIFFICULTY,
    GAME_COUNTDOWN,
    GAME_CREDITS,
    GAME_PLAYING,
    GAME_NEXT_LEVEL,
    GAME_OVER
};

typedef struct {
    unsigned char state;
    unsigned char level;

    unsigned long int instability;
    unsigned long int possibly_corrupted_blocks;
    float lag;
    unsigned long int steps;

    float lag_max;

    float lag_amount;

    float countdown;

    float task_animation_time;

    unsigned char preview;
    unsigned char ask_frees;
    unsigned char ask_reallocs;
    size_t region_count;

    size_t old_size;
    size_t alloc_size;

    unsigned char help_screen;

    Assets assets;
    Region regions[GAME_MAX_REGIONS];
    GC gc;

    struct {
        Meter corruption;
        Meter instability;
        Meter lag;
    } meters;
} Game;

int game_init(Game *game);
void game_logic(Game *game, float delta);
void game_draw(Game *game, float delta);
void game_free(Game *game);

#endif
