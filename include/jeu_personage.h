#ifndef PERSONAGE_H
#define PERSONAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Sprite sheet : grille 5x5 = 25 frames
 * Chaque frame : 256x256 pixels
 * col = frame % 5,  row = frame / 5
 * src.x = col*256,  src.y = row*256
 */
#define SPR_COLS      5
#define SPR_ROWS      5
#define SPR_FRAMES   25
#define FRAME_DELAY  80    /* ms par frame */
/* Hitbox physique (collision, spawn) — identique v4 */
#define PHYS_W       80
#define PHYS_H      160
/* Affichage sprite (visuel uniquement, x3 vs v4) */
#define DISPLAY_W   480
#define DISPLAY_H   480
/* Offset pour centrer le grand sprite sur la hitbox */
#define DISP_OFF_X  ((DISPLAY_W - PHYS_W) / 2)   /* 200 */
#define DISP_OFF_Y  (DISPLAY_H - PHYS_H)          /* 320 */

/* Physique */
#define GRAVITY       0.5
#define WALK_SPEED    5
#define RUN_SPEED     9
#define JUMP_VEL    -14.0
#define SUPER_JUMP  -22.0

typedef enum { A_IDLE=0,A_WALK,A_RUN,A_JUMP,A_ATTACK,A_DIE,A_COUNT } PAction;

typedef struct {
    SDL_Texture *sprites[A_COUNT];
    SDL_Texture *heart_tex;
    SDL_Rect     pos;       /* position dans le MONDE */
    double       vy;
    int          id;        /* 0=Jake 1=Neytiri */
    PAction      action;
    int          frame;
    Uint32       last_frame;
    int          facing_right;
    int          on_ground;
    int          is_jumping;
    int          is_attacking;
    int          hit_dealt;   /* 1 = degat deja inflige ce clic */
    int          invincible;
    Uint32       hurt_time;
    int          hp;
    int          score;
    int          bows_collected;
    int          alive;
} Personage;

SDL_Texture* perso_load_tex(const char *path, SDL_Renderer *r);
void perso_init  (Personage *p, int id, int x, int ground_y, SDL_Renderer *r);
void perso_update(Personage *p,
                  int right, int left, int run,
                  int jump, int super_jump, int attack,
                  SDL_Surface *mask, int ground_y,
                  int world_w, int world_h);
void perso_render    (SDL_Renderer *r, Personage *p,
                      int cam_x, int cam_y, int view_x, int view_w);
void perso_render_hud(SDL_Renderer *r, Personage *p,
                      int hud_x, TTF_Font *font);
void perso_free(Personage *p);

#endif
