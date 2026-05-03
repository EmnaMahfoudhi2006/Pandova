#include "../include/menu_score.h"

static void sort_scores(ScoreEntry *sc, int n)
{
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
            if (sc[j].score > sc[i].score) {
                ScoreEntry tmp = sc[i]; sc[i] = sc[j]; sc[j] = tmp;
            }
}

void enregistrer_score(MenuScore *ms, SDL_Renderer *r)
{
    if (ms->nb_scores < MAX_SCORES) {
        strncpy(ms->scores[ms->nb_scores].name, ms->nom, 49);
        ms->scores[ms->nb_scores].score = ms->score_joueur;
        ms->nb_scores++;
    } else {
        int min = 0;
        for (int i = 1; i < MAX_SCORES; i++)
            if (ms->scores[i].score < ms->scores[min].score) min = i;
        if (ms->score_joueur > ms->scores[min].score) {
            strncpy(ms->scores[min].name, ms->nom, 49);
            ms->scores[min].score = ms->score_joueur;
        }
    }
    sort_scores(ms->scores, ms->nb_scores);
    for (int i = 0; i < ms->nb_scores && i < MAX_SCORES; i++) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%d.  %s  —  %d pts",
                 i+1, ms->scores[i].name, ms->scores[i].score);
        txt_set(r, &ms->lignes[i], buf);
    }
}

void init_score(SDL_Renderer *r, MenuScore *ms)
{
    ms->bg_saisie = tex_load(r, P_SCORE "background1.jpg");
    ms->bg_scores = tex_load(r, P_SCORE "background2.jpg");
    ms->stars[0]  = tex_load(r, P_SCORE "star3.png");
    ms->stars[1]  = tex_load(r, P_SCORE "star2.png");
    ms->stars[2]  = tex_load(r, P_SCORE "star1.png");

    btn_init(r, &ms->valider, P_SCORE"valider.png", P_SCORE"valider_hover.png", 540, 430, 180, 60);
    btn_init(r, &ms->retour,  P_SCORE"retour.png",  P_SCORE"retour_hover.png",  460, 600, 160, 55);
    btn_init(r, &ms->quitter, P_SCORE"quitter.png", P_SCORE"quitter_hover.png", 660, 600, 160, 55);

    txt_init(r, &ms->label_titre,  430, 150, 30, 30, 120, P_SCORE"arial.ttf", 42, "Entrez votre pseudo :");
    txt_init(r, &ms->champ_nom,    540, 290, 20, 20,  20, P_SCORE"arial.ttf", 38, "");
    txt_init(r, &ms->titre_scores, 430, 140, 30, 30, 120, P_SCORE"arial.ttf", 44, "Meilleurs Scores");
    for (int i = 0; i < MAX_SCORES; i++)
        txt_init(r, &ms->lignes[i], 590, 300+i*110, 20, 20, 20, P_SCORE"arial.ttf", 34, "");

    ms->nb_scores = 0; ms->score_joueur = 100;
    ms->nom[0] = '\0'; ms->nom_len = 0;
    ms->page = 0; ms->music_lancee = 0;
}

void render_score(SDL_Renderer *r, MenuScore *ms, AudioCtx *audio)
{
    if (ms->page == 0) {
        if (ms->bg_saisie) SDL_RenderCopy(r, ms->bg_saisie, NULL, NULL);
        txt_render(r, &ms->label_titre);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 200);
        SDL_Rect box = {530, 285, 220, 50};
        SDL_RenderFillRect(r, &box);
        SDL_SetRenderDrawColor(r, 30, 30, 120, 255);
        SDL_RenderDrawRect(r, &box);
        txt_render(r, &ms->champ_nom);
        btn_render(r, &ms->valider);
    } else {
        if (!ms->music_lancee) { audio_switch(audio, 5); ms->music_lancee = 1; }
        if (ms->bg_scores) SDL_RenderCopy(r, ms->bg_scores, NULL, NULL);
        txt_render(r, &ms->titre_scores);
        for (int i = 0; i < ms->nb_scores && i < MAX_SCORES; i++) {
            if (ms->stars[i]) {
                SDL_Rect sr = {470, 290+i*110, 0, 0};
                SDL_QueryTexture(ms->stars[i], NULL, NULL, &sr.w, &sr.h);
                SDL_RenderCopy(r, ms->stars[i], NULL, &sr);
            }
            txt_render(r, &ms->lignes[i]);
        }
        btn_render(r, &ms->retour);
        btn_render(r, &ms->quitter);
    }
}

void free_score(MenuScore *ms)
{
    if (ms->bg_saisie) SDL_DestroyTexture(ms->bg_saisie);
    if (ms->bg_scores) SDL_DestroyTexture(ms->bg_scores);
    for (int i = 0; i < 3; i++) if (ms->stars[i]) SDL_DestroyTexture(ms->stars[i]);
    btn_free(&ms->valider); btn_free(&ms->retour); btn_free(&ms->quitter);
    txt_free(&ms->label_titre); txt_free(&ms->champ_nom); txt_free(&ms->titre_scores);
    for (int i = 0; i < MAX_SCORES; i++) txt_free(&ms->lignes[i]);
}

/* ── Sauvegarde un score dans le fichier ── */
void append_score_to_file(const char *name, int score, const char *path)
{
    FILE *f = fopen(path, "a");
    if (!f) return;
    fprintf(f, "%s %d\n", name, score);
    fclose(f);
}

/* ── Charge les scores depuis le fichier et trie les 3 meilleurs ── */
void load_scores_from_file(MenuScore *ms, SDL_Renderer *r, const char *path)
{
    /* Lire tous les scores du fichier */
    ScoreEntry all[100];
    int nb = 0;
    FILE *f = fopen(path, "r");
    if (f) {
        char name[50]; int score;
        while (nb < 100 && fscanf(f, "%49s %d", name, &score) == 2) {
            strncpy(all[nb].name, name, 49);
            all[nb].score = score;
            nb++;
        }
        fclose(f);
    }

    /* Trier décroissant */
    for (int i = 0; i < nb-1; i++)
        for (int j = i+1; j < nb; j++)
            if (all[j].score > all[i].score) {
                ScoreEntry tmp = all[i]; all[i] = all[j]; all[j] = tmp;
            }

    /* Garder les 3 meilleurs */
    ms->nb_scores = (nb > MAX_SCORES) ? MAX_SCORES : nb;
    for (int i = 0; i < ms->nb_scores; i++) {
        ms->scores[i] = all[i];
        char buf[128];
        snprintf(buf, sizeof(buf), "%d.  %s  -  %d pts",
                 i+1, ms->scores[i].name, ms->scores[i].score);
        txt_set(r, &ms->lignes[i], buf);
    }
}
