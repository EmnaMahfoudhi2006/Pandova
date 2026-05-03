#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

/* ── Dimensions monde ── */
#define BG_LVL1_W   15215
#define BG_LVL1_H    1092
#define BG_LVL2_W   15216
#define BG_LVL2_H    1082

/*
 * Sol visuel (Y où le haut de l'herbe commence)
 * Le joueur doit avoir ses pieds (pos.y + DISPLAY_H) = GROUND_Y
 */
#define GROUND_LVL1   952   /* haut de l'herbe LVL1 */
#define GROUND_LVL2   940   /* haut du sol LVL2     */

typedef struct {
    SDL_Texture *tex;
    SDL_Surface *mask;
    int          width, height;
    int          ground_y;   /* Y du sol pour ce niveau */
} Level;

typedef struct {
    Level levels[2];
    int   cur;
} Background;

/*
 * COLLISION PARFAITE — 8 points de référence (PDF ESPRIT)
 *  Retourne :
 *   0 = libre
 *   1 = NOIR  → obstacle/sol (bloque, peut marcher dessus)
 *   2 = BLANC → danger (perd 1 vie, doit sauter)
 *
 * NOTE: On teste UNIQUEMENT les points bas (pieds) pour le sol
 *       et les points latéraux pour les murs, afin d'éviter
 *       les faux positifs sur les plateformes.
 */
SDL_Color bg_get_pixel  (SDL_Surface *mask, int x, int y);
int       bg_collision  (SDL_Surface *mask, SDL_Rect pos);
int       bg_col_feet   (SDL_Surface *mask, SDL_Rect pos);   /* sol seulement */
int       bg_col_sides  (SDL_Surface *mask, SDL_Rect pos);   /* murs seulement */
int       bg_col_top    (SDL_Surface *mask, SDL_Rect pos);   /* plafond */

void bg_init        (Background *b, SDL_Renderer *r);
void bg_free        (Background *b);
void bg_switch_level(Background *b, int lvl);
void bg_render      (SDL_Renderer *r, Background *b,
                     int cam_x, int cam_y,
                     int dst_x, int dst_y, int dst_w, int dst_h);

#endif
