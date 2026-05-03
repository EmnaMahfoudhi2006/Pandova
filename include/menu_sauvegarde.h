#ifndef MENU_SAUVEGARDE_H
#define MENU_SAUVEGARDE_H

#include "header.h"

typedef struct {
    SDL_Texture *bg;
    Btn  oui, non;
    Btn  charger, nouvelle, retour;
    TxtLabel question;
    int  page;   /* 0=oui/non  1=charger/nouvelle */
} MenuSauvegarde;

void init_sauvegarde(SDL_Renderer *r, MenuSauvegarde *m);
void render_sauvegarde(SDL_Renderer *r, MenuSauvegarde *m);
void free_sauvegarde(MenuSauvegarde *m);

#endif
