#include <stdio.h>
#include <stdlib.h>

#include <gfx.h>
#include <game.h>

static Game game;

static int loop(float delta) {
    game_logic(&game, delta);
    game_draw(&game, delta);

    return 0;
}

int main(int argc, char **argv) {

    (void)argc;
    (void)argv;

    if(gfx_init("Garbage collector's minion")){
        return EXIT_FAILURE;
    }

    if(game_init(&game)){
        return EXIT_FAILURE;
    }

    gfx_mainloop(loop);

    game_free(&game);

    gfx_free();

    return EXIT_SUCCESS;
}
