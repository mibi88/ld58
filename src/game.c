#include <game.h>

#include <stdio.h>

#include <boundingbox.h>

#define ASSETS game->assets
#define SCALE ASSETS.scale

#define LEN(str) (sizeof(str)-1)
#define CHAR_H (ASSETS.font.h/8*SCALE)
#define CHAR_W (ASSETS.font.w/16*SCALE)
#define STR_W(str) (CHAR_W*LEN(str))
#define STR_X(str) ((gfx_w()-STR_W(str))/2+CHAR_W/2)

#define CENTER_STR(y, str) \
    gfx_text(&ASSETS.font, STR_X(str), y, str, 255, 255, 255, SCALE)

#define PLAY "PLAY"
#define PLAY_Y (gfx_h()/2-16*SCALE-CHAR_H)
#define HELP "HELP"
#define HELP_Y (gfx_h()/2-16*SCALE+CHAR_H)
#define CREDITS "2025 Mibi88    Made with <3 for the Ludum Dare 58!"
#define CREDITS_Y (gfx_h()-32*SCALE)

#define FAIL "FAIL ALLOCATION"
#define FAIL_X (gfx_w()-32*SCALE-STR_W(FAIL))

#define PLAY_AGAIN_Y (gfx_h()/2+64*SCALE)
#define TO_TITLE_Y (PLAY_AGAIN_Y+CHAR_H)

#define RETRY "RETRY"
#define NEXT_LEVEL "NEXT LEVEL"
#define TO_TITLE "BACK TO TITLE"

static void show_difficulty_1(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "Let's allocate some blocks, buddy!");
}

static void show_difficulty_2(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "Oh and btw free the memory when asked to!");
}

static void show_difficulty_3(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "What about realloc?");
}

static void show_difficulty_4(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "You'll have to count blocks yourself now, "
               "buddy! You're not a baby anymore!");
}

static void show_difficulty_5(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "Here, one more region to handle >:D!");
}

static void show_difficulty_6(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "Oh oh, the program is laggy, be faster or "
                                  "the user will terminate it!");
}

static void show_difficulty_7(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "It's too easy? Let's see...");
}

static void show_difficulty_8(Game *game) {
    CENTER_STR(GAME_DIFFICULTY_Y, "Be ready for some CHAOS!");
}

typedef struct {
    unsigned char preview;
    unsigned char ask_frees;
    unsigned char ask_reallocs;
    size_t alloc_max;

    float lag_max;

    size_t regions;
    unsigned char region_sizes[GAME_MAX_REGIONS];

    void (*show_difficulty)(Game *game);
} LevelConfig;

static LevelConfig configs[GAME_LEVEL_AMOUNT] = {
    {1, 0, 0, 50,  5,    1, {16},        show_difficulty_1},
    {1, 1, 0, 50,  5,    1, {16},        show_difficulty_2},
    {1, 1, 1, 50,  5,    1, {16},        show_difficulty_3},
    {0, 1, 1, 50,  5,    1, {16},        show_difficulty_4},
    {1, 1, 1, 50,  5,    2, {24, 16},    show_difficulty_5},
    {1, 1, 1, 50,  2,    3, {16, 24, 8}, show_difficulty_6},
    {1, 1, 1, 100, 0.75, 3, {16, 24, 8}, show_difficulty_7},
    {0, 1, 1, 100, 0.75, 3, {16, 24, 8}, show_difficulty_8}
};

static char *tutorial_screens[ASSETS_HELP_LEN] = {
    "assets/tutorial/requests.png",
    "assets/tutorial/allocate.png",
    "assets/tutorial/realloc.png",
    "assets/tutorial/fail_alloc.png",
    "assets/tutorial/free.png",
    "assets/tutorial/corruption.png",
    "assets/tutorial/lag.png"
};

static void level_end(Game *game) {
    size_t i;

    for(i=0;i<game->region_count;i++){
        game->regions[i].preview = 0;
    }
    game->state = GAME_NEXT_LEVEL;
}

static char task_buffer[256];

static void find_task(Game *game) {
    size_t size;
    size_t old_size;
    unsigned char task;

    long unsigned int pos;

    task = gc_find_task(&game->gc, &size, &old_size);

    printf("Task at %lu\n", game->gc.taskpos);

    switch(task){
        case GC_ALLOC:
            sprintf(task_buffer, "Allocate %lu blocks\n", size);
            break;
        case GC_REALLOC:
            pos = game->gc.allocation_pos[game->gc.taskpos];
            sprintf(task_buffer, "Resize %lu in region %lu from %lu blocks to "
                    "%lu blocks\n", pos&0xFF, (pos>>8)+1, old_size, size);
            game->old_size = old_size;
            break;
        case GC_FREE:
            pos = game->gc.allocation_pos[game->gc.taskpos];
            sprintf(task_buffer, "Free %lu in region %lu\n", pos&0xFF,
                    (pos>>8)+1);
            region_free_internal(game->regions+(pos>>8), pos&0xFF, size);
            break;
        case GC_DONE:
            level_end(game);
            break;
    }

    game->alloc_size = size;
    game->steps++;

    game->task_animation_time = GAME_TASK_ANIM_DURATION;
}

static void load_level(Game *game) {
    LevelConfig *config = configs+game->level;
    size_t i;
    size_t max_usage = 0;

    game->instability = 0;
    game->possibly_corrupted_blocks = 0;

    game->preview = config->preview;
    game->ask_frees = config->ask_frees;
    game->ask_reallocs = config->ask_reallocs;

    game->region_count = config->regions;

    game->lag_max = config->lag_max;

    for(i=0;i<config->regions;i++){
        max_usage += config->region_sizes[i];
        region_init(game->regions+i, &ASSETS, config->region_sizes[i],
                    game->preview);
    }

    game->alloc_size = 5;

    game->steps = 0;
    game->lag = 0;
    game->lag_amount = 0;

    game->countdown = 0;
    game->task_animation_time = GAME_TASK_ANIM_DURATION;

    gc_init(&game->gc, config->alloc_max, max_usage, config->ask_frees,
            config->ask_reallocs);

    find_task(game);
}

static void reset(Game *game) {
    game->level = GAME_FIRST_LEVEL;

    load_level(game);
}

int game_init(Game *game) {
    size_t i;

    if(gfx_load_texture(&ASSETS.tiles, "assets/tiles.png", 32, 32)){
        return 1;
    }

    if(gfx_load_texture(&ASSETS.font, "assets/font.png", 8*16, 10*8)){
        return 1;
    }

    if(gfx_load_texture(&ASSETS.menu, "assets/menu.png", 140, 240)){
        return 1;
    }

    if(gfx_load_texture(&ASSETS.well_done, "assets/well_done.png", 81, 13)){
        return 1;
    }

    if(gfx_load_texture(&ASSETS.game_over, "assets/game_over.png", 84, 13)){
        return 1;
    }

    for(i=0;i<ASSETS_HELP_LEN;i++){
        if(gfx_load_texture(ASSETS.help_screens+i, tutorial_screens[i], 1196,
                            719)){
            return 1;
        }
    }

    ASSETS.tile_size = 32;
    SCALE = 2;

    meter_init(&game->meters.instability, "INSTABILITY", GAME_METER_SIZE, 3);
    meter_init(&game->meters.corruption, "CORRUPTION", GAME_METER_SIZE, 3);
    meter_init(&game->meters.lag, "LAG", GAME_METER_SIZE, 3);

    reset(game);

    return 0;
}

void game_logic(Game *game, float delta) {
    size_t i;

    switch(game->state){
        case GAME_TITLE:
            if(gfx_clicked(GB_LEFT)){
                if(BB_IN(gfx_mouse_x(), gfx_mouse_y(), STR_X(PLAY)-CHAR_W/2,
                         PLAY_Y-CHAR_H/2, STR_W(PLAY), CHAR_H)){
                    game->state = GAME_DIFFICULTY;

                    reset(game);
                }
                if(BB_IN(gfx_mouse_x(), gfx_mouse_y(), STR_X(HELP)-CHAR_W/2,
                         HELP_Y-CHAR_H/2, STR_W(HELP), CHAR_H)){
                    game->state = GAME_HELP;
                    game->help_screen = 0;
                }
            }

            break;
        case GAME_HELP:
            if(gfx_clicked(GB_LEFT)){
                game->help_screen++;
                if(game->help_screen >= ASSETS_HELP_LEN){
                    game->state = GAME_TITLE;
                }
            }
            break;
        case GAME_CREDITS:
            if(gfx_clicked(GB_LEFT)){
                game->state = GAME_TITLE;
            }
            break;
        case GAME_DIFFICULTY:
            game->countdown += delta;
            if(game->countdown > 5 || gfx_clicked(GB_LEFT)){
                game->countdown = 0;
                game->state = GAME_COUNTDOWN;
            }
            break;
        case GAME_COUNTDOWN:
            game->countdown += delta;
            if(game->countdown > 3){
                game->state = GAME_PLAYING;
            }
            break;
        case GAME_PLAYING:
            game->lag += delta;

            game->lag_amount = game->lag*GAME_METER_SIZE/
                               (game->steps*game->lag_max);

            if(game->lag_amount > GAME_METER_SIZE){
                game->state = GAME_OVER;
            }
            if(xorshift(&game->gc.seed)%game->gc
               .allocation_max < game->instability){
                game->state = GAME_OVER;
            }
            if(xorshift(&game->gc.seed)%game->gc
               .max_usage < game->possibly_corrupted_blocks){
                game->state = GAME_OVER;
            }

            if(game->task_animation_time > 0){
                game->task_animation_time -= delta;
                if(game->task_animation_time < 0){
                    game->task_animation_time = 0;
                }
            }

            if(gfx_clicked(GB_LEFT)){
                if(game->gc.task == GC_FREE){
                    for(i=0;i<game->region_count;i++){
                        long int selected = game->regions[i].selected;
                        if(selected < 0) continue;
                        region_free_alloc(game->regions+i, selected,
                                          game->alloc_size);
                        gc_task_done(&game->gc, selected|(i<<8));
                        find_task(game);
                        game->regions[i].selected = -1;
                        break;
                    }
                }

                if(game->gc.task == GC_ALLOC || game->gc.task == GC_REALLOC){
                    for(i=0;i<game->region_count;i++){
                        long int selected = game->regions[i].selected;

                        if(selected < 0) continue;

                        if(game->gc.task == GC_REALLOC){
                            long unsigned int selected =
                                    game->gc.allocation_pos[game->gc.taskpos];

                            region_free_internal(game->regions+(selected>>8),
                                                 selected&0xFF,
                                                 game->alloc_size);
                            region_free_alloc(game->regions+(selected>>8),
                                              selected&0xFF, game->alloc_size);
                        }

                        if(!region_alloc(game->regions+i, game->alloc_size,
                                         &game->possibly_corrupted_blocks)){
                            gc_task_done(&game->gc, selected|(i<<8));
                            find_task(game);
                            break;
                        }
                    }
                }
            }
            break;
        case GAME_NEXT_LEVEL:
            if(gfx_clicked(GB_LEFT)){
                if(BB_IN(gfx_mouse_x(), gfx_mouse_y(), STR_X(NEXT_LEVEL),
                         PLAY_AGAIN_Y-CHAR_H/2, STR_W(NEXT_LEVEL), CHAR_H)){
                    game->level++;
                    if(game->level >= GAME_LEVEL_AMOUNT){
                        game->state = GAME_CREDITS;
                    }else{
                        load_level(game);
                        game->state = GAME_DIFFICULTY;
                    }
                }
                if(BB_IN(gfx_mouse_x(), gfx_mouse_y(), STR_X(TO_TITLE),
                         TO_TITLE_Y-CHAR_H/2, STR_W(TO_TITLE), CHAR_H)){
                    game->state = GAME_TITLE;
                }
            }
            break;
        case GAME_OVER:
            if(gfx_clicked(GB_LEFT)){
                if(BB_IN(gfx_mouse_x(), gfx_mouse_y(), STR_X(RETRY),
                         PLAY_AGAIN_Y-CHAR_H/2, STR_W(RETRY), CHAR_H)){
                    load_level(game);
                    game->state = GAME_COUNTDOWN;
                }
                if(BB_IN(gfx_mouse_x(), gfx_mouse_y(), STR_X(TO_TITLE),
                         TO_TITLE_Y-CHAR_H/2, STR_W(TO_TITLE), CHAR_H)){
                    game->state = GAME_TITLE;
                }
            }
            break;
        default:
            /* This should not happen */
            fputs("Invalid screen!\n", stderr);
            game->state = GAME_TITLE;
    }
}

void game_draw(Game *game, float delta) {
    int y = 64*SCALE;
    size_t i;

    char buffer[64];

    float scale;
    GFXTexture *texture;

    (void)delta;

    switch(game->state){
        case GAME_TITLE:
            CENTER_STR(32*SCALE, "Garbage collector's minion");
            CENTER_STR(PLAY_Y, PLAY);
            CENTER_STR(HELP_Y, HELP);
            CENTER_STR(CREDITS_Y, CREDITS);
            break;
        case GAME_HELP:
            /* HACK: My tutorial images contain 1 line of white pixels, so I'm
             *       cropping them here. */
            texture = ASSETS.help_screens+game->help_screen;
            scale = gfx_w()/(float)texture->w < gfx_h()/(float)(texture->h-1) ?
                    gfx_w()/(float)texture->w : gfx_h()/(float)(texture->h-1);
            gfx_subimage(texture, gfx_w()/2, gfx_h()/2,
                         texture->w*scale, (texture->h-1)*scale,
                         0, 1, texture->w, texture->h-1, 0, 255, 255, 255);

            CENTER_STR(16*SCALE, "Help - Click to continue...");
            break;
        case GAME_CREDITS:
            CENTER_STR(gfx_h()/2-CHAR_H*3, "THANKS FOR PLAYING!");
            CENTER_STR(gfx_h()/2-CHAR_H, "You've managed to beat all the "
                       "levels, you are a great memory allocator.");
            CENTER_STR(gfx_h()/2, "This game was made by Mibi88 for the Ludum "
                                  "Dare 58 (jam category).");
            CENTER_STR(gfx_h()/2+CHAR_H, "I hope that you've enjoyed playing "
                                         "it!");

            CENTER_STR(gfx_h()/2+CHAR_H*3, "Click to go back to the title "
                       "screen...");
            break;
        case GAME_DIFFICULTY:
            configs[game->level].show_difficulty(game);

            break;
        case GAME_OVER:
        case GAME_NEXT_LEVEL:
        case GAME_COUNTDOWN:
        case GAME_PLAYING:
            for(i=0;i<game->region_count;i++){
                sprintf(buffer, "Region %lu", (unsigned long int)i+1);
                gfx_text(&ASSETS.font, 32*SCALE, y, buffer, 255, 255, 255,
                         SCALE);
                y += CHAR_H;
                y = region_draw(game->regions+i, 48*SCALE, y,
                                gfx_w()-128*SCALE, game->alloc_size);
            }
#if 0
            printf("Possibly corrupted %lu - Instability %lu - Lag %lu\n",
                   game->possibly_corrupted_blocks, game->instability,
                   (unsigned long int)(game->lag/game->steps));
#endif

            meter_set_value(&game->meters.lag, game->lag_amount);
            meter_set_value(&game->meters.instability, game->instability);
            meter_set_value(&game->meters.corruption,
                            game->possibly_corrupted_blocks*
                            GAME_METER_SIZE/game->gc.max_usage);

            gfx_text(&ASSETS.font, gfx_w()-32*SCALE, 32*SCALE,
                     game->meters.lag.text, 255, 255, 255, SCALE);
            gfx_text(&ASSETS.font, gfx_w()-32*SCALE-CHAR_W, 32*SCALE,
                     game->meters.instability.text, 255, 255, 255, SCALE);
            gfx_text(&ASSETS.font, gfx_w()-32*SCALE-CHAR_W*2, 32*SCALE,
                     game->meters.corruption.text, 255, 255, 255, SCALE);

            y = gfx_h()-32*SCALE;
            if(game->gc.task != GC_FREE){
                gfx_text(&ASSETS.font, FAIL_X, y, FAIL, 255, 255, 255, SCALE);
                /* FIXME: Put this in game_logic in a clean way */
                if(gfx_clicked(GB_LEFT) && game->state == GAME_PLAYING &&
                   BB_IN(gfx_mouse_x(), gfx_mouse_y(), FAIL_X, y-CHAR_H/2,
                         STR_W(FAIL), CHAR_H)){
                    game->instability++;
                    gc_task_fail(&game->gc);
                    find_task(game);
                }
            }
            if(game->state == GAME_PLAYING){
                /* Show the action to do */
                unsigned char r = game->gc.task == GC_ALLOC ? 255 : 0;
                unsigned char g = game->gc.task == GC_FREE ? 255 :
                                  (game->gc.task == GC_REALLOC ? 180 : 0);
                unsigned char b = game->gc.task == GC_REALLOC ? 255 : 0;

                gfx_text(&ASSETS.font, 32*SCALE,
                         (32-(int)(game->task_animation_time*
                                   GAME_TASK_ANIM_OFFSET/
                                   GAME_TASK_ANIM_DURATION))*SCALE,
                         task_buffer, r, g, b, SCALE);
            }else{
                GFXTexture *message;

                if(game->state == GAME_COUNTDOWN){
                    gfx_subimage(&ASSETS.tiles, gfx_w()/2, gfx_h()/2,
                                 ASSETS.tile_size*SCALE,
                                 ASSETS.tile_size*SCALE,
                                 (int)game->countdown*ASSETS.tile_size,
                                 ASSETS.tile_size*2, ASSETS.tile_size,
                                 ASSETS.tile_size, 0, 255, 255, 255);
                    break;
                }

                /* Show some text and a button to go to the next level/retry */
                message = game->state == GAME_NEXT_LEVEL ?
                                      &ASSETS.well_done : &ASSETS.game_over;

                GFX_IMAGE_OPT(&ASSETS.menu, gfx_w()/2, gfx_h()/2,
                              ASSETS.menu.w*SCALE, ASSETS.menu.h*SCALE, 0);
                GFX_IMAGE_OPT(message, gfx_w()/2, gfx_h()/2-64*SCALE,
                              message->w*SCALE, message->h*SCALE, 0);

                if(game->state == GAME_OVER) CENTER_STR(PLAY_AGAIN_Y, RETRY);
                else CENTER_STR(PLAY_AGAIN_Y, NEXT_LEVEL);
                CENTER_STR(TO_TITLE_Y, TO_TITLE);
            }
            break;
        default:
            /* This should not happen */
            fputs("Invalid screen!\n", stderr);
            game->state = GAME_TITLE;
    }
}

void game_free(Game *game) {
    (void)game;
    /**/
}
