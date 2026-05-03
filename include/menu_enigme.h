#ifndef MENU_ENIGME_H
#define MENU_ENIGME_H

#include "header.h"

typedef struct {
    SDL_Texture *bg;
    SDL_Texture *panel;
    SDL_Texture *btnA, *btnA_h, *btnB, *btnB_h, *btnC, *btnC_h;
    SDL_Texture *quiz_btn, *quiz_btn_h;
    SDL_Texture *puzzle_btn, *puzzle_btn_h;
    SDL_Texture *heart, *game_over, *survived;
    TTF_Font    *font, *font_title, *font_small;

    Question  questions[MAX_QUESTIONS];
    int       nb_q;
    int       round[ROUND_SIZE];
    int       round_idx;
    int       current;

    int  hover_a, hover_b, hover_c;
    int  hover_quiz, hover_puzzle;
    int  view;
    int  fini;
    int  show_hearts;
    int  score;
    int  vies;
    int  corrects;
    int  total;

    Uint32 t_start;
    Uint32 t_max;
    int    temps_ecoule;
    int    flash;
    Uint32 flash_start;

    float  end_scale;
    double end_angle;
    Uint32 end_anim;
} MenuEnigme;

int  init_enigme(SDL_Renderer *r, MenuEnigme *e);
void render_enigme(SDL_Renderer *r, MenuEnigme *e);
void update_enigme(MenuEnigme *e);
void enigme_start(MenuEnigme *e);
void free_enigme(MenuEnigme *e);

#endif
