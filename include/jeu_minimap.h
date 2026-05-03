#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include "jeu_background.h"

#define MM_W  260
#define MM_H   70

typedef struct {
    int x, y;   /* position sur l'écran */
} MinimapPos;

/*
 * Affiche la minimap à la position (mm_x, mm_y) sur l'écran.
 * Montre le background, le joueur (blanc), les ennemis (rouge), les bows (jaune).
 */
void minimap_render(SDL_Renderer *r, Background *bg,
                    int mm_x, int mm_y,
                    int p1_wx, int p1_wy,       /* positions monde joueurs */
                    int p2_wx, int p2_wy,
                    int *enemy_x, int *enemy_y, int nb_enemies,
                    int *bow_x,   int *bow_y,   int *bow_col, int nb_bows);

#endif
