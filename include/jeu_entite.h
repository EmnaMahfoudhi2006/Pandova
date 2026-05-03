#ifndef ENTITE_H
#define ENTITE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HEARTS  4
#define MAX_BOWS    5

typedef struct {
    SDL_Rect pos;
    int      collected;
} Heart;

typedef struct {
    SDL_Rect pos;
    int      collected;
} Bow;

/* ===== PROTOTYPES ===== */
SDL_Texture* entite_load_tex(const char *path, SDL_Renderer *r);

void hearts_init(Heart *hearts, int nb, int *xs, int *ys);
void bows_init  (Bow   *bows,   int nb, int *xs, int *ys);

/* retourne 1 si collision et collecte */
int heart_check(Heart *h, SDL_Rect player);
int bow_check  (Bow   *b, SDL_Rect player);

void hearts_render(SDL_Renderer *r, Heart *hearts, int nb,
                   SDL_Texture *tex, int cam_x, int cam_y,
                   int view_x, int view_w);
void bows_render  (SDL_Renderer *r, Bow *bows, int nb,
                   SDL_Texture *tex, int cam_x, int cam_y,
                   int view_x, int view_w);

#endif
