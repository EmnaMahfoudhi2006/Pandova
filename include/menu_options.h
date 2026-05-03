#ifndef MENU_OPTIONS_H
#define MENU_OPTIONS_H

#include "header.h"

typedef struct {
    SDL_Texture *bg;
    Btn  diminuer, augmenter, normal, plein, retour;
} MenuOptions;

void init_options(SDL_Renderer *r, MenuOptions *m);
void render_options(SDL_Renderer *r, MenuOptions *m);
void free_options(MenuOptions *m);

#endif
