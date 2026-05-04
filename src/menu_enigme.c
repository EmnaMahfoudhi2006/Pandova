/**
 * @file menu_enigme.c
 * @brief Gestion du menu des énigmes dans le jeu SDL2.
 * @author Emna
 * @date 2026
 *
 * Ce fichier contient les fonctions responsables de l'initialisation,
 * la mise à jour, l'affichage et la libération des ressources du menu énigme.
 */

#include "../include/menu_enigme.h"

/**
 * @brief Démarre une nouvelle session d'énigmes.
 *
 * Cette fonction choisit aléatoirement les questions du round,
 * initialise le score, les vies, le temps et les variables d'état.
 *
 * @param e Pointeur vers la structure MenuEnigme.
 * @return void
 */
void enigme_start(MenuEnigme *e)
{
    int chosen[ROUND_SIZE]; int cnt = 0;
    while (cnt < ROUND_SIZE) {
        int rr = rand() % e->nb_q;
        int dup = 0;
        for (int i = 0; i < cnt; i++) if (chosen[i] == rr) { dup = 1; break; }
        if (!dup) chosen[cnt++] = rr;
    }
    for (int i = 0; i < ROUND_SIZE; i++) e->round[i] = chosen[i];
    e->round_idx = 0;
    e->current   = e->round[0];
    e->t_start   = SDL_GetTicks();
    e->t_max     = 20000;
    e->temps_ecoule = 0;
    e->flash     = 0;
    e->fini      = 0;
    e->score     = 0;
    e->vies      = 3;
    e->corrects  = 0;
    e->total     = 0;
    e->end_scale = 1.0f;
    e->end_angle = 0.0;
}

/**
 * @brief Met à jour l'état du menu énigme.
 *
 * Cette fonction gère le temps restant, la perte de vies,
 * le passage à la question suivante, l'animation de fin
 * et l'effet flash en cas d'erreur ou de temps écoulé.
 *
 * @param e Pointeur vers la structure MenuEnigme.
 * @return void
 */
void update_enigme(MenuEnigme *e)
{
    Uint32 now = SDL_GetTicks();
    if (e->view == 1 && !e->fini) {
        if (now - e->t_start >= e->t_max) {
            e->temps_ecoule = 1; e->vies--;
            e->flash = 1; e->flash_start = now;
            e->total++; e->round_idx++;
            if (e->vies <= 0)
                { e->fini=1; e->show_hearts=0; e->end_anim=now; }
            else if (e->round_idx >= ROUND_SIZE)
                { e->fini=1; e->show_hearts=1; e->end_anim=now; }
            else
                { e->current=e->round[e->round_idx]; e->t_start=now; e->temps_ecoule=0; }
        }
    }
    if (e->flash && now - e->flash_start > 250) e->flash = 0;
    if (e->view == 1 && e->fini) {
        float t = (now - e->end_anim) / 1000.0f;
        e->end_scale = 1.0f + 0.35f * sinf(t * 5.0f);
        e->end_angle += 4.0;
        if (e->end_angle >= 360.0) e->end_angle -= 360.0;
    }
}

/**
 * @brief Initialise le menu énigme.
 *
 * Cette fonction charge les textures, les polices et les questions
 * depuis le fichier questions.txt.
 *
 * @param r Pointeur vers le renderer SDL.
 * @param e Pointeur vers la structure MenuEnigme.
 * @return 1 si l'initialisation réussit, 0 sinon.
 */
int init_enigme(SDL_Renderer *r, MenuEnigme *e)
{
    memset(e, 0, sizeof(*e));
    e->bg           = tex_load(r, P_ENIGME "background_game.png");
    e->btnA         = tex_load(r, P_ENIGME "btn_a.png");
    e->btnA_h       = tex_load(r, P_ENIGME "btn_a_h.png");
    e->btnB         = tex_load(r, P_ENIGME "btn_b.png");
    e->btnB_h       = tex_load(r, P_ENIGME "btn_b_h.png");
    e->btnC         = tex_load(r, P_ENIGME "btn_c.png");
    e->btnC_h       = tex_load(r, P_ENIGME "btn_c_h.png");
    e->quiz_btn     = tex_load(r, P_ENIGME "quiz.png");
    e->quiz_btn_h   = tex_load(r, P_ENIGME "quiz_h.png");
    e->puzzle_btn   = tex_load(r, P_ENIGME "puzzle.png");
    e->puzzle_btn_h = tex_load(r, P_ENIGME "puzzle_h.png");
    e->heart        = tex_load(r, P_ENIGME "heart.png");
    e->game_over    = tex_load(r, P_ENIGME "GAME OVER.png");
    e->survived     = tex_load(r, P_ENIGME "YOUSERV.png");

    e->font       = TTF_OpenFont(P_ENIGME "arial.ttf", 26);
    e->font_title = TTF_OpenFont(P_ENIGME "arial.ttf", 40);
    e->font_small = TTF_OpenFont(P_ENIGME "arial.ttf", 20);
    if (!e->font || !e->font_title || !e->font_small) return 0;

    FILE *f = fopen(P_ENIGME "questions.txt", "r");
    if (!f) { SDL_Log("Cannot open questions.txt"); return 0; }
    char line[1024];
    while (fgets(line, sizeof(line), f) && e->nb_q < MAX_QUESTIONS) {
        Question *q = &e->questions[e->nb_q];
        line[strcspn(line, "\r\n")] = '\0';
        if (!line[0]) continue;
        int rc = sscanf(line, "%255[^?]?%127[^.].%127[^.].%127[^.].%d",
                        q->question, q->answers[0], q->answers[1], q->answers[2], &q->correct);
        if (rc == 5 && q->correct >= 0 && q->correct <= 2) e->nb_q++;
    }
    fclose(f);
    if (e->nb_q < ROUND_SIZE) { SDL_Log("Pas assez de questions"); return 0; }

    e->view = 0;
    srand((unsigned)time(NULL));
    return 1;
}

/**
 * @brief Affiche le menu énigme à l'écran.
 *
 * Cette fonction affiche soit le choix du mode, soit les questions,
 * les réponses, le score, la barre de temps et l'écran de fin.
 *
 * @param r Pointeur vers le renderer SDL.
 * @param e Pointeur vers la structure MenuEnigme.
 * @return void
 */
void render_enigme(SDL_Renderer *r, MenuEnigme *e)
{
    /* Zones definies via macros dans header.h — memes que les events */
    SDL_Rect quiz_r   = {E_QUIZ_X,   E_QUIZ_Y,   E_QW, E_QH};
    SDL_Rect puzzle_r = {E_PUZZLE_X, E_PUZZLE_Y, E_QW, E_QH};
    SDL_Rect aR = {E_BTN_X, E_AY, E_BW, E_BH};
    SDL_Rect bR = {E_BTN_X, E_BY, E_BW, E_BH};
    SDL_Rect cR = {E_BTN_X, E_CY, E_BW, E_BH};

    SDL_Color black  = {0,   0,   0,   255};
    SDL_Color yellow = {255, 220, 80,  255};
    SDL_Color red    = {220, 60,  60,  255};

    SDL_RenderClear(r);
    if (e->bg) SDL_RenderCopy(r, e->bg, NULL, NULL);

    if (e->view == 0) {
        if (e->panel) {
            SDL_Rect pr = {E_PANEL_X, E_PANEL_Y, E_PANEL_W, E_PANEL_H};
            SDL_RenderCopy(r, e->panel, NULL, &pr);
        }
        SDL_Surface *ts = TTF_RenderUTF8_Blended(e->font_title, "Choisissez votre mode", yellow);
        if (ts) {
            SDL_Texture *tt = SDL_CreateTextureFromSurface(r, ts);
            SDL_Rect tr = {E_PANEL_X+(E_PANEL_W-ts->w)/2, E_PANEL_Y+(E_PANEL_H-ts->h)/2, ts->w, ts->h};
            SDL_RenderCopy(r, tt, NULL, &tr);
            SDL_DestroyTexture(tt); SDL_FreeSurface(ts);
        }

        SDL_Rect dq = quiz_r;
        if (e->hover_quiz) { dq.x-=6; dq.y-=3; dq.w+=12; dq.h+=6; }
        SDL_RenderCopy(r, (e->hover_quiz && e->quiz_btn_h) ? e->quiz_btn_h : e->quiz_btn, NULL, &dq);

        SDL_Rect dp = puzzle_r;
        if (e->hover_puzzle) { dp.x-=6; dp.y-=3; dp.w+=12; dp.h+=6; }
        SDL_RenderCopy(r, (e->hover_puzzle && e->puzzle_btn_h) ? e->puzzle_btn_h : e->puzzle_btn, NULL, &dp);

    } else {
        if (!e->fini) {
            Question *q = &e->questions[e->current];
            float ratio = 1.0f - (float)(SDL_GetTicks()-e->t_start) / (float)e->t_max;
            if (ratio < 0) ratio = 0;

            char hud[64]; snprintf(hud, sizeof(hud), "Score: %d", e->score);
            draw_text(r, e->font_small, hud, 30, 20, black);

            SDL_Rect barBg = {30,50,200,14}, barFg = {30,50,(int)(200*ratio),14};
            SDL_SetRenderDrawColor(r, 50,50,50,255);   SDL_RenderFillRect(r, &barBg);
            SDL_SetRenderDrawColor(r, 20,200,120,255); SDL_RenderFillRect(r, &barFg);

            char prog[32]; snprintf(prog, sizeof(prog), "%d / %d", e->round_idx+1, ROUND_SIZE);
            draw_text(r, e->font_small, prog, WIN_W-100, 30, black);

            SDL_Surface *ts2 = TTF_RenderUTF8_Blended(e->font_title, "QUESTION", yellow);
            if (ts2) {
                SDL_Texture *tt = SDL_CreateTextureFromSurface(r, ts2);
                SDL_Rect tr = {E_PANEL_X+(E_PANEL_W-ts2->w)/2, E_PANEL_Y+15, ts2->w, ts2->h};
                SDL_RenderCopy(r, tt, NULL, &tr);
                SDL_DestroyTexture(tt); SDL_FreeSurface(ts2);
            }

            draw_text_wrap(r, e->font, q->question, E_PANEL_X+20, E_PANEL_Y+75, E_PANEL_W-40, black);

            SDL_Rect da = aR; if (e->hover_a) { da.x-=4; da.y-=2; da.w+=8; da.h+=4; }
            SDL_Rect db = bR; if (e->hover_b) { db.x-=4; db.y-=2; db.w+=8; db.h+=4; }
            SDL_Rect dc = cR; if (e->hover_c) { dc.x-=4; dc.y-=2; dc.w+=8; dc.h+=4; }

            SDL_RenderCopy(r, e->hover_a ? e->btnA_h : e->btnA, NULL, &da);
            SDL_RenderCopy(r, e->hover_b ? e->btnB_h : e->btnB, NULL, &db);
            SDL_RenderCopy(r, e->hover_c ? e->btnC_h : e->btnC, NULL, &dc);

            draw_text_wrap(r, e->font, q->answers[0], E_TXT_X, E_AY+(E_BH/2)-13, E_BLUE_W-(E_TXT_X-E_BLUE_X)-20, black);
            draw_text_wrap(r, e->font, q->answers[1], E_TXT_X, E_BY+(E_BH/2)-13, E_BLUE_W-(E_TXT_X-E_BLUE_X)-20, black);
            draw_text_wrap(r, e->font, q->answers[2], E_TXT_X, E_CY+(E_BH/2)-13, E_BLUE_W-(E_TXT_X-E_BLUE_X)-20, black);

        } else {
            if (e->show_hearts && e->heart)
                for (int i = 0; i < e->vies; i++) {
                    SDL_Rect hr = {25+i*50, 20, 42, 42};
                    SDL_RenderCopy(r, e->heart, NULL, &hr);
                }

            SDL_Texture *end = (e->vies > 0) ? e->survived : e->game_over;
            if (end) {
                int w, h; SDL_QueryTexture(end, NULL, NULL, &w, &h);
                SDL_Rect dst = {WIN_W/2-(int)(w*e->end_scale)/2,
                                WIN_H/2-20-(int)(h*e->end_scale)/2,
                                (int)(w*e->end_scale), (int)(h*e->end_scale)};
                double ang = (e->vies > 0) ? -e->end_angle : e->end_angle;
                SDL_RenderCopyEx(r, end, NULL, &dst, ang, NULL, SDL_FLIP_NONE);
            }

            char stats[128];
            snprintf(stats, sizeof(stats), "Score: %d   Correct: %d / %d",
                     e->score, e->corrects, ROUND_SIZE);
            draw_text(r, e->font, stats, 380, 520, black);
            draw_text(r, e->font_small, "Cliquez pour retourner au menu", 400, 570, black);
        }
    }

    if (e->flash && !e->fini) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 255, 0, 0, 80);
        SDL_RenderFillRect(r, NULL);
        if (e->temps_ecoule) draw_text(r, e->font_title, "TEMPS ECOULE!",     420, 310, red);
        else                 draw_text(r, e->font_title, "MAUVAISE REPONSE!", 360, 310, red);
    }
}

/**
 * @brief Libère les ressources du menu énigme.
 *
 * Cette fonction détruit les textures et ferme les polices utilisées
 * par le menu énigme afin d'éviter les fuites mémoire.
 *
 * @param e Pointeur vers la structure MenuEnigme.
 * @return void
 */
void free_enigme(MenuEnigme *e)
{
#define DT(x) if(x){SDL_DestroyTexture(x);x=NULL;}

    DT(e->btnA) DT(e->btnA_h) DT(e->btnB) DT(e->btnB_h)
    DT(e->btnC) DT(e->btnC_h)
    DT(e->quiz_btn) DT(e->quiz_btn_h)
    DT(e->puzzle_btn) DT(e->puzzle_btn_h)
    DT(e->heart) DT(e->game_over) DT(e->survived)
#undef DT

    if (e->font)       { TTF_CloseFont(e->font);       e->font = NULL; }
    if (e->font_title) { TTF_CloseFont(e->font_title); e->font_title = NULL; }
    if (e->font_small) { TTF_CloseFont(e->font_small); e->font_small = NULL; }
}
