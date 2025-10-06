#include <gfx.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define PI 3.141592653589793

#define W 1200
#define H 720

static unsigned char keys[GK_AMOUNT+1];

static const SDLKey sdl_keys[GK_AMOUNT] = {
    SDLK_UP,
    SDLK_DOWN,
    SDLK_LEFT,
    SDLK_RIGHT
};

static unsigned short int mouse_x = 0, mouse_y = 0;

static unsigned char buttons[GB_AMOUNT];
static unsigned char clicked[GB_AMOUNT];
static unsigned char released[GB_AMOUNT];

static const SDLKey sdl_buttons[GK_AMOUNT] = {
    SDL_BUTTON_LEFT,
    SDL_BUTTON_MIDDLE,
    SDL_BUTTON_RIGHT
};

static int window_w, window_h;

static int bpp;
static const int flags = SDL_OPENGL|SDL_RESIZABLE;

static void on_resize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);

    window_w = w;
    window_h = h;
}

int gfx_init(char *title) {
    /* Initialize SDL and OpenGL */
    const SDL_VideoInfo *info;

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        return 1;
    }

    info = SDL_GetVideoInfo();

    if(info == NULL){
        return 1;
    }

    bpp = info->vfmt->BitsPerPixel;

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if(!SDL_SetVideoMode(W, H, bpp, flags)){
        return 1;
    }

    SDL_WM_SetCaption(title, NULL);

    /* Setup OpenGL */

    glClearColor(0.2, 0.2, 0.2, 1);

    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    on_resize(W, H);

    memset(keys, 0, GK_AMOUNT);
    memset(buttons, 0, GB_AMOUNT);

    return 0;
}

void gfx_mainloop(int loop(float delta)) {
    unsigned long int last;
    unsigned long int delta;

    last = SDL_GetTicks();

    do{
        SDL_Event event;
        unsigned long int new;

        int has_resized = 0;
        int w, h;

        memset(clicked, 0, GB_AMOUNT);
        memset(released, 0, GB_AMOUNT);

        glClear(GL_COLOR_BUFFER_BIT);

        while(SDL_PollEvent(&event)){
            size_t i;
            switch(event.type){
                case SDL_KEYDOWN:
                    for(i=0;i<GK_AMOUNT;i++){
                        if(sdl_keys[i] == event.key.keysym.sym) keys[i] = 1;
                    }
                    break;
                case SDL_KEYUP:
                    for(i=0;i<GK_AMOUNT;i++){
                        if(sdl_keys[i] == event.key.keysym.sym) keys[i] = 0;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    for(i=0;i<GB_AMOUNT;i++){
                        if(sdl_buttons[i] == event.button.button){
                            buttons[i] = 1;
                            clicked[i] = 1;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    for(i=0;i<GB_AMOUNT;i++){
                        if(sdl_buttons[i] == event.button.button){
                            buttons[i] = 0;
                            released[i] = 1;
                        }
                    }
                    break;
                case SDL_VIDEORESIZE:
                    /* It is kinda broken for some reason */
                    w = event.resize.w;
                    h = event.resize.h;
                    has_resized = 1;
                    break;
                case SDL_QUIT:
                    SDL_Quit();
                    return;
            }
        }

        if(has_resized){
            printf("%d, %d\n", w, h);
            if(SDL_SetVideoMode(w, h, bpp, flags)){
                on_resize(w, h);
            }else{
                fputs("Resizing failed!\n", stderr);
            }
        }

        do{
            new = SDL_GetTicks();
            delta = new-last;
        }while(delta < 16);
        last = new;

        if(loop(delta/1e3)) break;
        SDL_GL_SwapBuffers();

    }while(1);
}

int gfx_load_texture(GFXTexture *texture, char *file, int w, int h) {
    SDL_Surface *image;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    image = IMG_Load(file);

    if(image == NULL){
        return 1;
    }

    texture->w = w;
    texture->h = h;

    texture->rw = image->w;
    texture->rh = image->h;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image->pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(image);

    return 0;
}

void gfx_subimage(GFXTexture *texture, int x, int y, int w, int h, int rx,
                  int ry, int rw, int rh, int a, int r, int g, int b) {
    float u1 = rx/(float)texture->rw;
    float v1 = ry/(float)texture->rh;
    float u2 = (rx+rw)/(float)texture->rw;
    float v2 = (ry+rh)/(float)texture->rh;

    glLoadIdentity();

    glTranslatef(x, y, 0);
    glRotatef(a*180/PI, 0, 0, 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColor3f(r/255.0, g/255.0, b/255.0);

    glBegin(GL_QUADS);

    glTexCoord2f(u1, v1);
    glVertex2f(-w/2, -h/2);
    glTexCoord2f(u2, v1);
    glVertex2f(w/2, -h/2);
    glTexCoord2f(u2, v2);
    glVertex2f(w/2, h/2);
    glTexCoord2f(u1, v2);
    glVertex2f(-w/2, h/2);

    glEnd();
}

void gfx_text(GFXTexture *font, int x, int y, char *str, int r, int g, int b,
              float scale) {
    char c;
    int sx = x;

    unsigned int w = font->w/16;
    unsigned int h = font->h/8;

    unsigned int rw = w*scale;
    unsigned int rh = h*scale;

    while((c = *str++)){
        if(c == '\n'){
            x = sx;
            y += rh;
            continue;
        }
        gfx_subimage(font, x, y, rw, rh, (c&15)*w, (c>>4)*h, w, h, 0, r, g, b);
        x += rw;
    }
}

unsigned char gfx_keydown(GFXKey key) {
    return keys[key];
}

unsigned char gfx_button(GFXButton button) {
    return buttons[button];
}

unsigned char gfx_clicked(GFXButton button) {
    return clicked[button];
}

unsigned char gfx_released(GFXButton button) {
    return released[button];
}

int gfx_w(void) {
    return window_w;
}

int gfx_h(void) {
    return window_h;
}

int gfx_mouse_x(void) {
    return mouse_x;
}

int gfx_mouse_y(void) {
    return mouse_y;
}

void gfx_free(void) {
    /**/
}
