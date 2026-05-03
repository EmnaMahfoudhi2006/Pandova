#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ===== FENETRE ===== */
#define WIN_W        1280
#define WIN_H         720
#define MAX_SCORES      3
#define MAX_QUESTIONS  32
#define ROUND_SIZE      3

/* ===== CHEMINS ASSETS ===== */
#define P_PRINCIPAL  "assets/principal/"
#define P_OPTIONS    "assets/options/"
#define P_SAUVEGARDE "assets/sauvegarde/"
#define P_JOUEUR     "assets/joueur/"
#define P_SCORE      "assets/score/"
#define P_ENIGME     "assets/enigme/"

/* ===== ENIGME LAYOUT — coordonnees uniques render+events ===== */
#define E_PANEL_X   230
#define E_PANEL_Y   128
#define E_PANEL_W   790
#define E_PANEL_H   185
#define E_BLUE_X    270
#define E_BLUE_Y    380
#define E_BLUE_W    660
#define E_BLUE_H    340
#define E_QW        220
#define E_QH         80
#define E_QUIZ_X    (E_BLUE_X + E_BLUE_W/2 - E_QW - 25)
#define E_QUIZ_Y    (E_BLUE_Y + E_BLUE_H/2 - E_QH/2)
#define E_PUZZLE_X  (E_BLUE_X + E_BLUE_W/2 + 25)
#define E_PUZZLE_Y  (E_BLUE_Y + E_BLUE_H/2 - E_QH/2)
#define E_BTN_X     (E_BLUE_X + 30)
#define E_TXT_X     (E_BLUE_X + 135)
#define E_BW        100
#define E_BH         60
#define E_AY        (E_BLUE_Y + 20)
#define E_BY        (E_BLUE_Y + 110)
#define E_CY        (E_BLUE_Y + 200)

/* ===== ETATS ===== */
typedef enum {
    STATE_MAIN,
    STATE_OPTIONS,
    STATE_SAUVEGARDE,
    STATE_JOUEUR,
    STATE_SCORE,
    STATE_ENIGME
} GameState;

/* ===== BOUTON ===== */
typedef struct {
    SDL_Texture *tex;
    SDL_Texture *tex_h;
    SDL_Rect     dst;
    int          hovered;
} Btn;

/* ===== LABEL TEXTE ===== */
typedef struct {
    SDL_Rect     rect;
    TTF_Font    *font;
    SDL_Texture *texture;
    SDL_Color    color;
    char         txt[256];
} TxtLabel;

/* ===== SCORE ===== */
typedef struct {
    char name[50];
    int  score;
} ScoreEntry;

/* ===== QUESTION ===== */
typedef struct {
    char question[256];
    char answers[3][128];
    int  correct;
} Question;

/* ===== CONTEXTE AUDIO ===== */
typedef struct {
    Mix_Music  *principal_music;
    Mix_Music  *options_music;
    Mix_Music  *sauvegarde_music;
    Mix_Music  *joueur_music;
    Mix_Music  *score_music;
    Mix_Music  *enigme_music;
    Mix_Chunk  *hover_snd;
    Mix_Chunk  *click_snd;
    int         current;
    int         volume;
} AudioCtx;

/* ===== UTILITAIRES ===== */
SDL_Texture* tex_load(SDL_Renderer *r, const char *path);
void btn_init(SDL_Renderer *r, Btn *b, const char *p, const char *ph, int x, int y, int w, int h);
void btn_render(SDL_Renderer *r, Btn *b);
int  btn_hit(Btn *b, int x, int y);
void btn_free(Btn *b);
void txt_init(SDL_Renderer *r, TxtLabel *t, int x, int y,
              Uint8 cr, Uint8 cg, Uint8 cb, const char *font, int sz, const char *txt);
void txt_set(SDL_Renderer *r, TxtLabel *t, const char *txt);
void txt_render(SDL_Renderer *r, TxtLabel *t);
void txt_free(TxtLabel *t);
void audio_switch(AudioCtx *a, int which);
void hover_once(AudioCtx *a, int *last, int cur);
void draw_text(SDL_Renderer *r, TTF_Font *f, const char *txt, int x, int y, SDL_Color c);
void draw_text_wrap(SDL_Renderer *r, TTF_Font *f, const char *txt, int x, int y, int w, SDL_Color c);

#endif
