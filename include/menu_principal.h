#ifndef MENU_PRINCIPAL_H
#define MENU_PRINCIPAL_H

#include "header.h"

typedef struct {
    SDL_Texture *bg;
    SDL_Texture *logo;
    Btn  jouer, options, score, histoire, quitter;
} MenuPrincipal;

void init_principal(SDL_Renderer *r, MenuPrincipal *m);
void render_principal(SDL_Renderer *r, MenuPrincipal *m);
void free_principal(MenuPrincipal *m);

#endif
