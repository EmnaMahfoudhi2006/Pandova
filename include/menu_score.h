#ifndef MENU_SCORE_H
#define MENU_SCORE_H

#include "header.h"

typedef struct {
    SDL_Texture *bg_saisie;
    SDL_Texture *bg_scores;
    SDL_Texture *stars[3];
    Btn          valider, retour, quitter;
    TxtLabel     label_titre;
    TxtLabel     champ_nom;
    TxtLabel     titre_scores;
    TxtLabel     lignes[MAX_SCORES];
    ScoreEntry   scores[MAX_SCORES];
    int          nb_scores;
    char         nom[50];
    int          nom_len;
    int          score_joueur;
    int          page;
    int          music_lancee;
} MenuScore;

void init_score(SDL_Renderer *r, MenuScore *ms);
void render_score(SDL_Renderer *r, MenuScore *ms, AudioCtx *audio);
void free_score(MenuScore *ms);
void enregistrer_score(MenuScore *ms, SDL_Renderer *r);
/* Charge les scores depuis scores.txt et met à jour l'affichage */
void load_scores_from_file(MenuScore *ms, SDL_Renderer *r, const char *path);
/* Sauvegarde un score directement dans scores.txt */
void append_score_to_file(const char *name, int score, const char *path);

#endif
