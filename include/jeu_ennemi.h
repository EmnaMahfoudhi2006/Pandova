#ifndef ENNEMI_H
#define ENNEMI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Hitbox physique ennemi */
#define PHYS_E      100
/* Affichage ennemi (x3) */
#define DISPLAY_E   450
#define DISP_OFF_EX ((DISPLAY_E - PHYS_E) / 2)   /* 175 */
#define DISP_OFF_EY (DISPLAY_E - PHYS_E)          /* 350 */

typedef enum { TYPE_ROBOT=0, TYPE_HUMAN=1 } EType;
typedef enum { E_IDLE=0,E_WALK,E_RUN,E_ATTACK,E_HIT,E_DIE,E_COUNT } EAction;

/* Info sprite par action */
typedef struct { const char *path; int cols; int rows; } SprInfo;

typedef struct {
    SDL_Texture *sprites[E_COUNT];
    int          cols[E_COUNT];
    int          rows[E_COUNT];

    SDL_Rect  pos;
    double    vy;
    int       on_ground;
    int       alive;
    int       health, max_hp;
    int       dir;
    EAction   action;
    EType     type;
    float     speed;
    int       frame;
    Uint32    last_frame;
    Uint32    last_attack;
    int       attack_cd;
} Ennemi;

SDL_Texture* ennemi_load_tex(const char *path, SDL_Renderer *r);
void ennemi_init  (Ennemi *e, EType type, int x, int y, SDL_Renderer *r);
void ennemi_update(Ennemi *e, SDL_Rect t1, SDL_Rect t2,
                   SDL_Surface *mask, int ground_y, int world_w, int world_h);
void ennemi_render(SDL_Renderer *r, Ennemi *e, int cam_x, int cam_y,
                   int view_x, int view_w);
void ennemi_hit   (Ennemi *e);
void ennemi_free  (Ennemi *e);

#endif
