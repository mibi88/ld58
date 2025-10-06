#ifndef GFX_H
#define GFX_H

typedef enum {
    GK_UP,
    GK_DOWN,
    GK__LEFT,
    GK_RIGHT,

    GK_AMOUNT
} GFXKey;

typedef enum {
    GB_LEFT,
    GB_MIDDLE,
    GB_RIGHT,

    GB_AMOUNT
} GFXButton;

typedef struct {
    unsigned int w, h;
    unsigned int rw, rh;
    unsigned int id;
} GFXTexture;

int gfx_init(char *title);

void gfx_mainloop(int loop(float delta));


void gfx_subimage(GFXTexture *texture, int x, int y, int w, int h, int rx,
                  int ry, int rw, int rh, int a, int r, int g, int b);
#define GFX_IMAGE_OPT(texture, x, y, width, height, a) \
    gfx_subimage(texture, x, y, width, height, 0, 0, (texture)->w, \
                 (texture)->h, a, 255, 255, 255)
#define GFX_IMAGE(texture, x, y) GFX_IMAGE_OPT(texture, x, y, (texture)->w, \
                                               (texture)->h, 0)
void gfx_text(GFXTexture *font, int x, int y, char *str, int r, int g, int b,
              float scale);
int gfx_load_texture(GFXTexture *texture, char *file, int w, int h);


unsigned char gfx_keydown(GFXKey key);
unsigned char gfx_button(GFXButton button);
unsigned char gfx_clicked(GFXButton button);
unsigned char gfx_released(GFXButton button);
int gfx_w(void);
int gfx_h(void);
int gfx_mouse_x(void);
int gfx_mouse_y(void);
void gfx_free(void);


void gfx_free(void);

#endif
