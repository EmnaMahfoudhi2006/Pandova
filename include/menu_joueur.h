#ifndef MENU_JOUEUR_H
#define MENU_JOUEUR_H

#include "header.h"

typedef struct {
    SDL_Texture *bg;
    Btn  mono, multi;
    Btn  avatar1, avatar2;
    Btn  input1, input2;
    Btn  valider, retour;
    int  page;
    int  mode;
    int  sel_avatar;
    int  sel_input;
} MenuJoueur;

void init_joueur(SDL_Renderer *r, MenuJoueur *m);
void render_joueur(SDL_Renderer *r, MenuJoueur *m, TTF_Font *f);
void free_joueur(MenuJoueur *m);

#endif
