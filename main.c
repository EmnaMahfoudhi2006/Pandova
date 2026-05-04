/*
 * ================================================================
 *  PROJET INTEGRE FINAL — main.c
 *
 *  FLUX :
 *   Menu Principal
 *    ├─ JOUER → Sauvegarde
 *    │    ├─ OUI → Charger → reprend depuis save.txt
 *    │    └─ NON → Nouveau → Mono/Multi → Nom → JEU
 *    ├─ OPTIONS
 *    ├─ SCORES
 *    └─ QUITTER
 *
 *  PENDANT LE JEU :
 *   ESC → sauvegarde auto dans save.txt + retour menu
 *   Mort joueur → ENIGME (Quiz ou Puzzle)
 *     Gagner enigme → +vies → continue
 *     Perdre enigme → GAME OVER → score enregistré
 *   Fin de niveau → score enregistré dans scores.txt
 *
 *  CONTROLES :
 *   Jake    : flèches ←→, ↑ saut, RALT super jump, RCTRL attaque
 *   Neytiri : Q/D marche, Z saut, TAB super jump, E attaque
 * ================================================================
 */

/* ── Headers menu ── */
#include "include/header.h"
#include "include/menu_principal.h"
#include "include/menu_options.h"
#include "include/menu_sauvegarde.h"
#include "include/menu_joueur.h"
#include "include/menu_score.h"
#include "include/menu_enigme.h"

/* ── Header puzzle ── */
#include "include/puzzle.h"

/* ── Headers jeu ── */
#include "include/jeu_personage.h"
#include "include/jeu_ennemi.h"
#include "include/jeu_background.h"
#include "include/jeu_minimap.h"
#include "include/jeu_entite.h"

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

/* ================================================================
 *  CONSTANTES JEU
 * ================================================================ */
#define WIN_W      1280
#define WIN_H       720
#define NB_ROBOTS     2
#define NB_HUMANS     3
#define NB_HEARTS     4
#define NB_BOWS       5

/* Positions items dans le monde — modifier ici */
static int HEART_X[NB_HEARTS] = {1500, 4000,  7000, 11000};
static int HEART_Y[NB_HEARTS] = { 780,  780,   780,   780};
static int BOW_X  [NB_BOWS]   = { 800, 2500,  5000,  8000, 12000};
static int BOW_Y  [NB_BOWS]   = { 200,  200,   200,   200,   200};
static int ROBOT_X [NB_ROBOTS] = {3500, 8000};
static int ROBOT_Y [NB_ROBOTS] = {   0,    0};  /* 0 = sol auto */
static int HUMAN_X [NB_HUMANS] = {3000, 7000, 11000};
static int HUMAN_Y [NB_HUMANS] = {   0,    0,     0};

/* Sols par niveau */
#define GROUND_LVL1  952
#define GROUND_LVL2  940

typedef enum { MODE_MONO=1, MODE_MULTI=2 } GameMode;
typedef enum { RES_NEXT, RES_GAMEOVER, RES_WIN, RES_QUIT, RES_ESCAPE } LvlRes;

/* ================================================================
 *  SAUVEGARDE / CHARGEMENT
 * ================================================================ */
typedef struct {
    int   valid;
    int   level;       /* 0 ou 1 */
    int   mode;        /* 1=mono 2=multi */
    int   char_id;
    int   hp1, hp2;
    int   score1, score2;
    int   pos_x1, pos_y1;
    int   pos_x2, pos_y2;
    int   bows1, bows2;
    char  name1[50];
    char  name2[50];
} SaveData;

static void save_game(SaveData *sd, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "valid=%d\n",   1);
    fprintf(f, "level=%d\n",   sd->level);
    fprintf(f, "mode=%d\n",    sd->mode);
    fprintf(f, "char_id=%d\n", sd->char_id);
    fprintf(f, "hp1=%d\n",     sd->hp1);
    fprintf(f, "hp2=%d\n",     sd->hp2);
    fprintf(f, "score1=%d\n",  sd->score1);
    fprintf(f, "score2=%d\n",  sd->score2);
    fprintf(f, "pos_x1=%d\n",  sd->pos_x1);
    fprintf(f, "pos_y1=%d\n",  sd->pos_y1);
    fprintf(f, "pos_x2=%d\n",  sd->pos_x2);
    fprintf(f, "pos_y2=%d\n",  sd->pos_y2);
    fprintf(f, "bows1=%d\n",   sd->bows1);
    fprintf(f, "bows2=%d\n",   sd->bows2);
    fprintf(f, "name1=%s\n",   sd->name1);
    fprintf(f, "name2=%s\n",   sd->name2);
    fclose(f);
}

static int load_save(SaveData *sd, const char *path)
{
    memset(sd, 0, sizeof(*sd));
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fscanf(f, "valid=%d\n",   &sd->valid);
    fscanf(f, "level=%d\n",   &sd->level);
    fscanf(f, "mode=%d\n",    &sd->mode);
    fscanf(f, "char_id=%d\n", &sd->char_id);
    fscanf(f, "hp1=%d\n",     &sd->hp1);
    fscanf(f, "hp2=%d\n",     &sd->hp2);
    fscanf(f, "score1=%d\n",  &sd->score1);
    fscanf(f, "score2=%d\n",  &sd->score2);
    fscanf(f, "pos_x1=%d\n",  &sd->pos_x1);
    fscanf(f, "pos_y1=%d\n",  &sd->pos_y1);
    fscanf(f, "pos_x2=%d\n",  &sd->pos_x2);
    fscanf(f, "pos_y2=%d\n",  &sd->pos_y2);
    fscanf(f, "bows1=%d\n",   &sd->bows1);
    fscanf(f, "bows2=%d\n",   &sd->bows2);
    fscanf(f, "name1=%49s\n", sd->name1);
    fscanf(f, "name2=%49s\n", sd->name2);
    fclose(f);
    return sd->valid;
}

/* save_score : délègue à append_score_to_file de menu_score.c */
#define save_score(n,s,p) append_score_to_file(n,s,p)

/* ================================================================
 *  GAME STATE
 * ================================================================ */
typedef struct {
    Personage  player;
    Ennemi     enemies[NB_HUMANS];
    int        nb_enemies;
    Heart      hearts[NB_HEARTS];
    Bow        bows[NB_BOWS];
    SDL_Texture *heart_tex;
    SDL_Texture *bow_tex;
    int        level;
    int        bows_total;
    int        game_over;
    int        level_done;
    int        enigme_triggered;   /* 1 = énigme ouverte après mort */
} JeuState;

typedef struct { int x, y; } Camera;

static void cam_update(Camera *c, SDL_Rect p, int ww, int wh, int vw)
{
    c->x = p.x - vw/2;
    c->y = p.y - WIN_H/2;
    if (c->x < 0) c->x = 0;
    if (c->y < 0) c->y = 0;
    if (c->x + vw > ww) c->x = ww - vw;
    if (c->y + WIN_H > wh) c->y = wh - WIN_H;
}

static int bb(SDL_Rect a, SDL_Rect b)
{ return !(a.x+a.w<b.x||a.x>b.x+b.w||a.y+a.h<b.y||a.y>b.y+b.h); }

/* ── Init GameState ── */
static void gs_init(JeuState *gs, int char_id, int lvl,
                    Background *bg, SDL_Renderer *r,
                    SaveData *sd)  /* NULL = nouveau jeu */
{
    memset(gs, 0, sizeof(*gs));
    gs->level = lvl;
    Level *lv = &bg->levels[lvl];
    int gy = lv->ground_y;
    int ww = lv->width;

    int sx = 300, sy_spawn = gy;
    int hp = 3;
    if (sd && sd->valid) {
        sx       = sd->pos_x1;
        sy_spawn = sd->pos_y1 + PHYS_H; /* recalcul ground */
        hp       = sd->hp1;
    }

    perso_init(&gs->player, char_id, sx, sy_spawn, r);
    gs->player.hp    = hp;
    gs->player.score = sd ? sd->score1 : 0;
    gs->player.bows_collected = sd ? sd->bows1 : 0;

    /* Ennemis */
    if (lvl == 0) {
        gs->nb_enemies = NB_ROBOTS;
        for (int i=0;i<NB_ROBOTS;i++)
            ennemi_init(&gs->enemies[i], TYPE_ROBOT,
                        ROBOT_X[i], ROBOT_Y[i]>0?ROBOT_Y[i]:gy-PHYS_E, r);
    } else {
        gs->nb_enemies = NB_HUMANS;
        for (int i=0;i<NB_HUMANS;i++)
            ennemi_init(&gs->enemies[i], TYPE_HUMAN,
                        HUMAN_X[i], HUMAN_Y[i]>0?HUMAN_Y[i]:gy-PHYS_E, r);
    }

    /* Hearts */
    gs->heart_tex = perso_load_tex("assets/jeu/hearts/heart.png", r);
    hearts_init(gs->hearts, NB_HEARTS, HEART_X, HEART_Y);

    /* Bows */
    if (lvl == 0) {
        gs->bow_tex = perso_load_tex("assets/jeu/bow/bow.png", r);
        bows_init(gs->bows, NB_BOWS, BOW_X, BOW_Y);
        gs->bows_total = sd ? sd->bows1 : 0;
    }
    (void)ww;
}

static void gs_free(JeuState *gs)
{
    perso_free(&gs->player);
    for (int i=0;i<gs->nb_enemies;i++) ennemi_free(&gs->enemies[i]);
    if (gs->heart_tex) { SDL_DestroyTexture(gs->heart_tex); gs->heart_tex=NULL; }
    if (gs->bow_tex)   { SDL_DestroyTexture(gs->bow_tex);   gs->bow_tex=NULL;   }
}

/* ── Texte utilitaire ── */
static void draw_txt(SDL_Renderer *r, TTF_Font *f,
                     const char *t, SDL_Color c, int x, int y)
{
    if (!f) return;
    SDL_Surface *s = TTF_RenderText_Blended(f, t, c);
    if (!s) return;
    SDL_Texture *tx = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect d={x,y,s->w,s->h};
    SDL_RenderCopy(r,tx,NULL,&d);
    SDL_DestroyTexture(tx); SDL_FreeSurface(s);
}
static void draw_txt_c(SDL_Renderer *r, TTF_Font *f,
                        const char *t, SDL_Color c, int cx, int y)
{
    if (!f) return;
    SDL_Surface *s = TTF_RenderText_Blended(f, t, c);
    if (!s) return;
    SDL_Texture *tx = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect d={cx-s->w/2,y,s->w,s->h};
    SDL_RenderCopy(r,tx,NULL,&d);
    SDL_DestroyTexture(tx); SDL_FreeSurface(s);
}

/* ================================================================
 *  PUZZLE — wrapper pour l'intégration
 *  Retourne : 1=gagné  0=perdu
 * ================================================================ */
static int run_puzzle(SDL_Renderer *r_menu, SDL_Window *win_menu)
{
    /* Le puzzle crée sa propre fenêtre fullscreen */
    (void)r_menu; (void)win_menu;

    PuzzleGame puzzle;
    if (!initSDL(&puzzle)) return 0;
    if (!loadGame(&puzzle)) { cleanupGame(&puzzle); return 0; }

    while (puzzle.running) {
        renderGame(&puzzle);
        handleInput(&puzzle);
        updateGame(&puzzle);
        if (puzzle.gameWin || puzzle.gameOver) {
            /* Laisser l'animation se jouer 2s */
            Uint32 t = SDL_GetTicks();
            while (SDL_GetTicks()-t < 2000) {
                renderGame(&puzzle);
                SDL_Delay(16);
                SDL_Event ev;
                while (SDL_PollEvent(&ev))
                    if (ev.type==SDL_QUIT||
                        (ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE))
                        goto done;
            }
            break;
        }
    }
done:;
    int won = puzzle.gameWin;
    cleanupGame(&puzzle);
    return won;
}

/* ================================================================
 *  ENIGME MORT — ouvre quiz ou puzzle selon choix joueur
 *  Retourne : vies gagnées (0 = perdu)
 * ================================================================ */
static int run_death_enigme(SDL_Renderer *r, SDL_Window *win,
                             TTF_Font *font, TTF_Font *fsm,
                             MenuEnigme *me, AudioCtx *audio)
{
    /* Écran choix */
    SDL_Event ev; int choice=0;
    while (!choice) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type==SDL_QUIT) return 0;
            if (ev.type==SDL_KEYDOWN) {
                if (ev.key.keysym.sym==SDLK_1) choice=1; /* quiz */
                if (ev.key.keysym.sym==SDLK_2) choice=2; /* puzzle */
                if (ev.key.keysym.sym==SDLK_ESCAPE) return 0;
            }
        }
        SDL_SetRenderDrawColor(r,10,10,30,255); SDL_RenderClear(r);
        SDL_Color gold={255,200,0,255}, white={255,255,255,255};
        draw_txt_c(r,font,"VOUS ETES MORT !",gold,WIN_W/2,150);
        draw_txt_c(r,fsm,"Relevez le defi pour gagner des vies !",white,WIN_W/2,250);
        draw_txt_c(r,fsm,"1 - QUIZ   (gagner = +2 vies)",white,WIN_W/2,340);
        draw_txt_c(r,fsm,"2 - PUZZLE (gagner = +3 vies)",white,WIN_W/2,400);
        draw_txt_c(r,fsm,"ESC - Abandonner (Game Over)",white,WIN_W/2,480);
        SDL_RenderPresent(r); SDL_Delay(16);
    }

    if (choice==1) {
        /* Quiz existant */
        me->view=1;
        audio_switch(audio,6);
        enigme_start(me);
        SDL_Event qev;
        int done=0;
        while (!done) {
            while (SDL_PollEvent(&qev)) {
                if (qev.type==SDL_QUIT) return 0;
                if (qev.type==SDL_KEYDOWN&&qev.key.keysym.sym==SDLK_ESCAPE)
                    return 0;
                /* Clics quiz */
                if (qev.type==SDL_MOUSEBUTTONDOWN) {
                    int mx=qev.button.x, my_=qev.button.y;
                    me->hover_a=(mx>=E_BTN_X&&mx<=E_BTN_X+E_BW&&my_>=E_AY&&my_<=E_AY+E_BH);
                    me->hover_b=(mx>=E_BTN_X&&mx<=E_BTN_X+E_BW&&my_>=E_BY&&my_<=E_BY+E_BH);
                    me->hover_c=(mx>=E_BTN_X&&mx<=E_BTN_X+E_BW&&my_>=E_CY&&my_<=E_CY+E_BH);
                    int ans=me->hover_a?0:me->hover_b?1:me->hover_c?2:-1;
                    if (ans>=0) {
                        Question *q=&me->questions[me->current];
                        if (ans==q->correct) { me->score+=10; me->corrects++; }
                        else me->vies--;
                        me->total++; me->round_idx++;
                        if (me->vies<=0||me->round_idx>=ROUND_SIZE)
                            { me->fini=1; done=1; }
                        else {
                            me->current=me->round[me->round_idx];
                            me->t_start=SDL_GetTicks();
                        }
                    }
                }
            }
            update_enigme(me);
            SDL_SetRenderDrawColor(r,0,0,0,255); SDL_RenderClear(r);
            render_enigme(r,me);
            SDL_RenderPresent(r); SDL_Delay(16);
            if (me->fini) done=1;
        }
        audio_switch(audio,0);
        /* Gagné si corrects >= ROUND_SIZE/2 */
        return (me->corrects >= ROUND_SIZE/2) ? 2 : 0;
    } else {
        /* Puzzle */
        int won = run_puzzle(r, win);
        return won ? 3 : 0;
    }
}

/* ================================================================
 *  RENDU VUE JEU
 * ================================================================ */
static void gs_render_view(SDL_Renderer *r, JeuState *gs, Background *bg,
                            Camera *cam, int view_x, int view_w,
                            TTF_Font *fsm, int world_w)
{
    Level *lv = &bg->levels[gs->level];
    bg_render(r, bg, cam->x, cam->y, view_x, 0, view_w, WIN_H);

    hearts_render(r,gs->hearts,NB_HEARTS,gs->heart_tex,cam->x,cam->y,view_x,view_w);
    if (gs->level==0)
        bows_render(r,gs->bows,NB_BOWS,gs->bow_tex,cam->x,cam->y,view_x,view_w);
    for (int i=0;i<gs->nb_enemies;i++)
        ennemi_render(r,&gs->enemies[i],cam->x,cam->y,view_x,view_w);
    perso_render(r,&gs->player,cam->x,cam->y,view_x,view_w);

    /* Minimap haut droite */
    {
        int ex[NB_HUMANS],ey[NB_HUMANS],bx[NB_BOWS],by_[NB_BOWS],bc[NB_BOWS];
        for (int i=0;i<gs->nb_enemies;i++){ex[i]=gs->enemies[i].pos.x;ey[i]=gs->enemies[i].pos.y;}
        for (int i=0;i<NB_BOWS;i++){
            bx[i]=(gs->level==0)?gs->bows[i].pos.x:0;
            by_[i]=(gs->level==0)?gs->bows[i].pos.y:0;
            bc[i]=(gs->level==0)?gs->bows[i].collected:1;
        }
        minimap_render(r,bg,view_x+view_w-MM_W-5,5,
                       gs->player.pos.x,gs->player.pos.y,
                       gs->player.pos.x,gs->player.pos.y,
                       ex,ey,gs->nb_enemies,bx,by_,bc,NB_BOWS);
    }

    perso_render_hud(r,&gs->player,view_x+8,fsm);

    /* Bows HUD */
    if (gs->level==0&&gs->bow_tex) {
        for (int i=0;i<NB_BOWS;i++){
            SDL_SetTextureAlphaMod(gs->bow_tex,gs->bows[i].collected?255:60);
            SDL_Rect br={view_x+8+i*32,WIN_H-42,26,26};
            SDL_RenderCopy(r,gs->bow_tex,NULL,&br);
        }
        SDL_SetTextureAlphaMod(gs->bow_tex,255);
    }

    /* Alerte bows */
    if (gs->level==0&&gs->player.pos.x>world_w-600&&gs->bows_total<NB_BOWS&&fsm){
        SDL_Color red={255,60,60,255};
        char buf[32]; snprintf(buf,sizeof(buf),"Bows: %d/5",gs->bows_total);
        draw_txt_c(r,fsm,buf,red,view_x+view_w/2,WIN_H/2);
    }
    (void)lv;
}

/* ================================================================
 *  UPDATE JEU
 * ================================================================ */
static void gs_update(JeuState *gs, Background *bg,
                       int right,int left,int run,int jump,int sjump,int attack)
{
    /* Ne pas bloquer si enigme en cours (elle peut ressusciter le joueur) */
    if (gs->game_over||gs->level_done) return;
    if (gs->enigme_triggered) return;
    Level *lv=&bg->levels[gs->level];
    int gy=lv->ground_y, ww=lv->width, wh=lv->height;

    perso_update(&gs->player,right,left,run,jump,sjump,attack,
                 lv->mask,gy,ww,wh);

    /* Joueur mort → déclencher énigme (pas de game_over direct) */
    if (!gs->player.alive || gs->player.hp<=0) {
        gs->player.hp=0;
        gs->player.alive=0;
        if (!gs->enigme_triggered) gs->enigme_triggered=1;
        return;
    }

    /* Ennemis */
    SDL_Rect dummy={-9999,-9999,1,1};
    for (int i=0;i<gs->nb_enemies;i++)
        ennemi_update(&gs->enemies[i],gs->player.pos,dummy,lv->mask,gy,ww,wh);

    /* Collision joueur ↔ ennemis */
    Uint32 now=SDL_GetTicks();
    for (int i=0;i<gs->nb_enemies;i++){
        if (!gs->enemies[i].alive) continue;
        /* Zone d'attaque distance (500px) */
        int ar=500;
        SDL_Rect az;
        if (gs->player.facing_right) az=(SDL_Rect){gs->player.pos.x,gs->player.pos.y,ar,gs->player.pos.h};
        else                          az=(SDL_Rect){gs->player.pos.x-ar,gs->player.pos.y,ar,gs->player.pos.h};

        if (gs->player.is_attacking&&!gs->player.hit_dealt&&
            gs->player.frame>=8&&bb(az,gs->enemies[i].pos)){
            ennemi_hit(&gs->enemies[i]);
            gs->player.hit_dealt=1;
            gs->player.score+=100;
            if (!gs->enemies[i].alive&&gs->player.hp<5) gs->player.hp++;
        }
        if (gs->enemies[i].action==E_ATTACK&&
            now-gs->enemies[i].last_attack>(Uint32)gs->enemies[i].attack_cd&&
            !gs->player.invincible&&bb(gs->player.pos,gs->enemies[i].pos)){
            gs->enemies[i].last_attack=now;
            gs->player.hp--;
            gs->player.invincible=1;
            gs->player.hurt_time=now;
            if (gs->player.hp<=0) {
                gs->player.alive=0;
                gs->enigme_triggered=1;
            }
        }
    }

    /* Hearts */
    for (int i=0;i<NB_HEARTS;i++)
        if (heart_check(&gs->hearts[i],gs->player.pos)&&gs->player.hp<5)
            {gs->player.hp++;gs->player.score+=50;}

    /* Bows */
    if (gs->level==0)
        for (int i=0;i<NB_BOWS;i++)
            if (bow_check(&gs->bows[i],gs->player.pos))
                {gs->player.bows_collected++;gs->bows_total++;gs->player.score+=200;}

    /* Fin de niveau */
    if (gs->player.pos.x>ww-400){
        if (gs->level==0&&gs->bows_total<NB_BOWS) {/* bloqué */}
        else gs->level_done=1;
    }
}

/* ================================================================
 *  BOUCLE NIVEAU
 * ================================================================ */
static LvlRes run_level(SDL_Renderer *r, SDL_Window *win,
                         TTF_Font *font, TTF_Font *fsm,
                         Background *bg, int lvl,
                         JeuState *gs1, JeuState *gs2,
                         GameMode mode, int char_id,
                         MenuEnigme *me, AudioCtx *audio,
                         const char *save_path)
{
    Level *lv=&bg->levels[lvl];
    int ww=lv->width, wh=lv->height;
    int vw=(mode==MODE_MULTI)?WIN_W/2:WIN_W;
    Camera cam1={0,0}, cam2={0,0};
    SDL_Event ev;

    /* Titre niveau */
    {
        char t[32]; snprintf(t,sizeof(t),"LEVEL %d",lvl+1);
        SDL_SetRenderDrawColor(r,0,0,0,255); SDL_RenderClear(r);
        SDL_Color gold={255,200,0,255};
        draw_txt_c(r,font,t,gold,WIN_W/2,WIN_H/2-30);
        SDL_RenderPresent(r); SDL_Delay(1500);
    }

    while (1) {
        /* Events */
        while (SDL_PollEvent(&ev)) {
            if (ev.type==SDL_QUIT) return RES_QUIT;
            if (ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE) {
                /* Sauvegarde automatique */
                SaveData sd={0};
                sd.valid=1; sd.level=lvl; sd.mode=mode; sd.char_id=char_id;
                sd.hp1=gs1->player.hp; sd.score1=gs1->player.score;
                sd.pos_x1=gs1->player.pos.x; sd.pos_y1=gs1->player.pos.y;
                sd.bows1=gs1->player.bows_collected;
                if (mode==MODE_MULTI&&gs2) {
                    sd.hp2=gs2->player.hp; sd.score2=gs2->player.score;
                    sd.pos_x2=gs2->player.pos.x; sd.pos_y2=gs2->player.pos.y;
                    sd.bows2=gs2->player.bows_collected;
                }
                save_game(&sd, save_path);
                return RES_ESCAPE;
            }
        }

        const Uint8 *k=SDL_GetKeyboardState(NULL);

        /* Input Jake */
        int j_r=k[SDL_SCANCODE_RIGHT], j_l=k[SDL_SCANCODE_LEFT];
        int j_run=k[SDL_SCANCODE_RSHIFT], j_jump=k[SDL_SCANCODE_UP];
        int j_sjump=k[SDL_SCANCODE_RALT], j_atk=k[SDL_SCANCODE_C];

        /* Input Neytiri */
        int n_r=k[SDL_SCANCODE_D], n_l=k[SDL_SCANCODE_Q]||k[SDL_SCANCODE_A];
        int n_run=k[SDL_SCANCODE_LSHIFT], n_jump=k[SDL_SCANCODE_Z]||k[SDL_SCANCODE_W];
        int n_sjump=k[SDL_SCANCODE_TAB], n_atk=k[SDL_SCANCODE_E];

        /* Update */
        if (mode==MODE_MONO) {
            int r2=0,l2=0,ru=0,ju=0,sj=0,at=0;
            if (char_id==0){r2=j_r;l2=j_l;ru=j_run;ju=j_jump;sj=j_sjump;at=j_atk;}
            else           {r2=n_r;l2=n_l;ru=n_run;ju=n_jump;sj=n_sjump;at=n_atk;}
            gs_update(gs1,bg,r2,l2,ru,ju,sj,at);
        } else {
            gs_update(gs1,bg,j_r,j_l,j_run,j_jump,j_sjump,j_atk);
            if (gs2) gs_update(gs2,bg,n_r,n_l,n_run,n_jump,n_sjump,n_atk);
        }

        /* Énigme si mort */
        if (gs1->enigme_triggered) {
            gs1->enigme_triggered=0;
            int vies = run_death_enigme(r,win,font,fsm,me,audio);
            if (vies>0) {
                /* Gagné → ressuscite le joueur et continue le jeu */
                gs1->player.hp    = vies;
                gs1->player.alive = 1;
                gs1->player.invincible = 1;
                gs1->player.hurt_time  = SDL_GetTicks();
                gs1->game_over = 0;   /* annuler tout game over */
            } else {
                /* Perdu → game over définitif */
                gs1->game_over = 1;
            }
        }
        if (gs2&&gs2->enigme_triggered) {
            gs2->enigme_triggered=0;
            int vies=run_death_enigme(r,win,font,fsm,me,audio);
            if (vies>0) {
                gs2->player.hp    = vies;
                gs2->player.alive = 1;
                gs2->player.invincible = 1;
                gs2->player.hurt_time  = SDL_GetTicks();
                gs2->game_over = 0;
            } else {
                gs2->game_over = 1;
            }
        }

        /* Caméras */
        cam_update(&cam1,gs1->player.pos,ww,wh,vw);
        if (gs2) cam_update(&cam2,gs2->player.pos,ww,wh,vw);

        /* Rendu */
        SDL_SetRenderDrawColor(r,0,0,0,255); SDL_RenderClear(r);
        gs_render_view(r,gs1,bg,&cam1,0,vw,fsm,ww);
        if (mode==MODE_MULTI&&gs2) {
            gs_render_view(r,gs2,bg,&cam2,vw,vw,fsm,ww);
            SDL_SetRenderDrawColor(r,200,200,200,255);
            SDL_RenderDrawLine(r,vw,0,vw,WIN_H);
        }
        SDL_RenderPresent(r); SDL_Delay(16);

        /* Vérifier fin */
        int p1done=gs1->game_over||gs1->level_done;
        int p2done=(mode==MODE_MONO)||!gs2||(gs2->game_over||gs2->level_done);
        if (!p1done) continue;
        if (!p2done) continue;
        if (gs1->level_done||(gs2&&gs2->level_done)) return RES_NEXT;
        return RES_GAMEOVER;
    }
}

/* ================================================================
 *  MENU CHOIX MODE (mono/multi)
 * ================================================================ */
static GameMode menu_mode_select(SDL_Renderer *r, TTF_Font *font, TTF_Font *fsm)
{
    SDL_Event ev; GameMode ch=0;
    while (!ch) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type==SDL_QUIT) return 0;
            if (ev.type==SDL_KEYDOWN) {
                if (ev.key.keysym.sym==SDLK_1) ch=MODE_MONO;
                if (ev.key.keysym.sym==SDLK_2) ch=MODE_MULTI;
                if (ev.key.keysym.sym==SDLK_ESCAPE) return 0;
            }
            if (ev.type==SDL_MOUSEBUTTONDOWN)
                ch=(ev.button.y<WIN_H/2)?MODE_MONO:MODE_MULTI;
        }
        SDL_SetRenderDrawColor(r,10,15,30,255); SDL_RenderClear(r);
        SDL_Color gold={255,200,0,255},white={255,255,255,255};
        draw_txt_c(r,font,"MODE DE JEU",gold,WIN_W/2,150);
        SDL_SetRenderDrawColor(r,40,80,40,255);
        SDL_Rect b1={WIN_W/2-180,250,360,80}; SDL_RenderFillRect(r,&b1);
        draw_txt_c(r,font,"1 - MONO",white,WIN_W/2,270);
        SDL_SetRenderDrawColor(r,40,40,80,255);
        SDL_Rect b2={WIN_W/2-180,370,360,80}; SDL_RenderFillRect(r,&b2);
        draw_txt_c(r,font,"2 - MULTI",white,WIN_W/2,390);
        draw_txt_c(r,fsm,"ESC = Retour",white,WIN_W/2,500);
        SDL_RenderPresent(r); SDL_Delay(16);
    }
    return ch;
}

/* ── Saisie nom ── */
static void menu_name(SDL_Renderer *r, TTF_Font *font, TTF_Font *fsm,
                       char *name, int maxlen, const char *label)
{
    name[0]='\0'; int len=0;
    SDL_StartTextInput();
    SDL_Event ev;
    int done=0;
    while (!done) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type==SDL_QUIT) { done=1; break; }
            if (ev.type==SDL_KEYDOWN) {
                if (ev.key.keysym.sym==SDLK_RETURN&&len>0) done=1;
                if (ev.key.keysym.sym==SDLK_BACKSPACE&&len>0)
                    name[--len]='\0';
            }
            if (ev.type==SDL_TEXTINPUT&&len<maxlen-1) {
                strcat(name,ev.text.text);
                len=(int)strlen(name);
            }
        }
        SDL_SetRenderDrawColor(r,10,15,30,255); SDL_RenderClear(r);
        SDL_Color gold={255,200,0,255},white={255,255,255,255};
        draw_txt_c(r,font,label,gold,WIN_W/2,180);
        SDL_SetRenderDrawColor(r,50,50,80,255);
        SDL_Rect box={WIN_W/2-150,290,300,50}; SDL_RenderFillRect(r,&box);
        SDL_SetRenderDrawColor(r,200,200,200,255); SDL_RenderDrawRect(r,&box);
        draw_txt_c(r,fsm,name,white,WIN_W/2,300);
        draw_txt_c(r,fsm,"ENTREE pour confirmer",white,WIN_W/2,400);
        SDL_RenderPresent(r); SDL_Delay(16);
    }
    SDL_StopTextInput();
    if (len==0) strcpy(name,"Joueur");
}

/* ================================================================
 *  LANCER UN JEU COMPLET (nouveau ou chargé)
 * ================================================================ */
static void launch_game(SDL_Renderer *r, SDL_Window *win,
                         TTF_Font *font, TTF_Font *fsm,
                         Background *bg, MenuEnigme *me,
                         AudioCtx *audio,
                         GameMode mode, int char_id,
                         SaveData *sd,
                         const char *save_path,
                         const char *scores_path)
{
    char name1[50]="Jake", name2[50]="Neytiri";
    if (!sd || !sd->valid) {
        menu_name(r,font,fsm,name1,50,"Nom Joueur 1");
        if (mode==MODE_MULTI)
            menu_name(r,font,fsm,name2,50,"Nom Joueur 2");
    } else {
        strncpy(name1,sd->name1,49);
        strncpy(name2,sd->name2,49);
    }

    int start_lvl = (sd&&sd->valid) ? sd->level : 0;

    JeuState gs1, gs2_data;
    JeuState *gs2 = (mode==MODE_MULTI) ? &gs2_data : NULL;

    bg_switch_level(bg,start_lvl);
    gs_init(&gs1, char_id, start_lvl, bg, r, sd);
    if (gs2) {
        SaveData sd2={0};
        if (sd&&sd->valid) {
            sd2=*sd; sd2.hp1=sd->hp2; sd2.score1=sd->score2;
            sd2.pos_x1=sd->pos_x2; sd2.pos_y1=sd->pos_y2;
            sd2.bows1=sd->bows2;
        }
        gs_init(gs2, 1, start_lvl, bg, r, sd&&sd->valid?&sd2:NULL);
    }

    int cur_lvl=start_lvl;
    LvlRes res=RES_GAMEOVER;

    for (cur_lvl=start_lvl; cur_lvl<2; cur_lvl++) {
        bg_switch_level(bg,cur_lvl);
        res=run_level(r,win,font,fsm,bg,cur_lvl,
                      &gs1,gs2,mode,char_id,me,audio,save_path);

        if (res==RES_QUIT||res==RES_ESCAPE) goto done;
        if (res==RES_GAMEOVER) break;

        /* Préparer niveau suivant */
        if (cur_lvl<1) {
            int hp1=gs1.player.hp, sc1=gs1.player.score;
            int hp2=gs2?gs2->player.hp:3, sc2=gs2?gs2->player.score:0;
            int bw1=gs1.player.bows_collected, bw2=gs2?gs2->player.bows_collected:0;
            gs_free(&gs1);
            if (gs2) gs_free(gs2);
            gs_init(&gs1,char_id,1,bg,r,NULL);
            gs1.player.hp=hp1; gs1.player.score=sc1; gs1.player.bows_collected=bw1;
            if (gs2) {
                gs_init(gs2,1,1,bg,r,NULL);
                gs2->player.hp=hp2; gs2->player.score=sc2; gs2->player.bows_collected=bw2;
            }
        }
    }

    /* Enregistrer score final */
    save_score(name1, gs1.player.score, scores_path);
    if (gs2) save_score(name2, gs2->player.score, scores_path);

    /* Écran fin */
    {
        const char *msg=(res==RES_NEXT||res==RES_WIN)?"VICTOIRE !":"GAME OVER";
        SDL_Event ev; int wait=1;
        while (wait) {
            while (SDL_PollEvent(&ev))
                if (ev.type==SDL_QUIT||ev.type==SDL_KEYDOWN||ev.type==SDL_MOUSEBUTTONDOWN)
                    wait=0;
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r,0,0,0,200); SDL_RenderFillRect(r,NULL);
            SDL_Color gold={255,200,0,255},white={255,255,255,255};
            draw_txt_c(r,font,msg,gold,WIN_W/2,200);
            char buf[64];
            snprintf(buf,sizeof(buf),"%s: %d pts",name1,gs1.player.score);
            draw_txt_c(r,fsm,buf,white,WIN_W/2,320);
            if (gs2){
                snprintf(buf,sizeof(buf),"%s: %d pts",name2,gs2->player.score);
                draw_txt_c(r,fsm,buf,white,WIN_W/2,370);
            }
            draw_txt_c(r,fsm,"Appuyez pour continuer",white,WIN_W/2,500);
            SDL_RenderPresent(r); SDL_Delay(16);
        }
    }

done:
    gs_free(&gs1);
    if (gs2) gs_free(gs2);
}

/* ================================================================
 *  MAIN
 * ================================================================ */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048);
    SDL_StartTextInput();

    SDL_Window *window = SDL_CreateWindow("Avatar Game",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        WIN_W,WIN_H,SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,
        SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

    /* ── Fonts ── */
    TTF_Font *font = TTF_OpenFont(P_SAUVEGARDE"arial.ttf",48);
    TTF_Font *fsm  = TTF_OpenFont(P_SAUVEGARDE"arial.ttf",24);

    /* ── Audio menu ── */
    AudioCtx audio={0};
    audio.principal_music =Mix_LoadMUS(P_PRINCIPAL"music.mp3");
    audio.options_music   =Mix_LoadMUS(P_OPTIONS"music.mp3");
    audio.sauvegarde_music=Mix_LoadMUS(P_SAUVEGARDE"music.mp3");
    audio.joueur_music    =Mix_LoadMUS(P_JOUEUR"music.mp3");
    audio.score_music     =Mix_LoadMUS(P_SCORE"score_music.mp3");
    audio.enigme_music    =Mix_LoadMUS(P_ENIGME"quiz_music.mp3");
    audio.hover_snd       =Mix_LoadWAV(P_SAUVEGARDE"hover.wav");
    audio.click_snd       =Mix_LoadWAV(P_SAUVEGARDE"click.wav");
    audio.volume          =MIX_MAX_VOLUME/2;
    Mix_VolumeMusic(audio.volume);
    audio_switch(&audio,1);

    /* ── Menus ── */
    MenuPrincipal  mp; init_principal(renderer,&mp);
    MenuOptions    mo; init_options(renderer,&mo);
    MenuSauvegarde ms; init_sauvegarde(renderer,&ms);
    MenuJoueur     mj; init_joueur(renderer,&mj);
    MenuScore      mc; init_score(renderer,&mc);
    MenuEnigme     me; int enigme_ok=init_enigme(renderer,&me);

    /* ── Background jeu ── */
    Background bg; bg_init(&bg,renderer);

    /* ── Fichiers sauvegarde ── */
    const char *save_path   = "save.txt";
    const char *scores_path = "scores.txt";

    TTF_Font *font_shared=TTF_OpenFont(P_SAUVEGARDE"arial.ttf",22);

    GameState state_menu=STATE_MAIN;
    int running=1;
    int last_hover=-1;
    SDL_Event ev;

    while (running) {
        int mx,my;
        SDL_GetMouseState(&mx,&my);

        while (SDL_PollEvent(&ev)) {
            if (ev.type==SDL_QUIT){running=0;break;}

            /* ── MENU PRINCIPAL ── */
            if (state_menu==STATE_MAIN) {
                mp.jouer.hovered    =btn_hit(&mp.jouer,mx,my);
                mp.options.hovered  =btn_hit(&mp.options,mx,my);
                mp.score.hovered    =btn_hit(&mp.score,mx,my);
                mp.histoire.hovered =btn_hit(&mp.histoire,mx,my);
                mp.quitter.hovered  =btn_hit(&mp.quitter,mx,my);
                int cur=mp.jouer.hovered?0:mp.options.hovered?1:mp.score.hovered?2:
                        mp.histoire.hovered?3:mp.quitter.hovered?4:-1;
                hover_once(&audio,&last_hover,cur);
                if (ev.type==SDL_KEYDOWN) {
                    if (ev.key.keysym.sym==SDLK_ESCAPE) running=0;
                    if (ev.key.keysym.sym==SDLK_j){state_menu=STATE_SAUVEGARDE;ms.page=0;audio_switch(&audio,3);}
                    if (ev.key.keysym.sym==SDLK_o){state_menu=STATE_OPTIONS;audio_switch(&audio,2);}
                    if (ev.key.keysym.sym==SDLK_m){
                        state_menu=STATE_SCORE;mc.music_lancee=0;
                        audio_switch(&audio,2);
                        load_scores_from_file(&mc,renderer,scores_path);
                        mc.page=1;
                    }
                }
                if (ev.type==SDL_MOUSEBUTTONDOWN) {
                    if (audio.click_snd) Mix_PlayChannel(-1,audio.click_snd,0);
                    if (mp.jouer.hovered)   {state_menu=STATE_SAUVEGARDE;ms.page=0;audio_switch(&audio,3);}
                    if (mp.options.hovered) {state_menu=STATE_OPTIONS;audio_switch(&audio,2);}
                    if (mp.score.hovered)   {
                        state_menu=STATE_SCORE;mc.page=0;mc.music_lancee=0;
                        audio_switch(&audio,2);
                        /* Charger les meilleurs scores depuis scores.txt */
                        load_scores_from_file(&mc,renderer,scores_path);
                        mc.page=1; /* Afficher directement le classement */
                    }
                    if (mp.quitter.hovered) running=0;
                }
            }

            /* ── OPTIONS ── */
            else if (state_menu==STATE_OPTIONS) {
                mo.diminuer.hovered =btn_hit(&mo.diminuer,mx,my);
                mo.augmenter.hovered=btn_hit(&mo.augmenter,mx,my);
                mo.normal.hovered   =btn_hit(&mo.normal,mx,my);
                mo.plein.hovered    =btn_hit(&mo.plein,mx,my);
                mo.retour.hovered   =btn_hit(&mo.retour,mx,my);
                int cur=mo.diminuer.hovered?0:mo.augmenter.hovered?1:mo.normal.hovered?2:
                        mo.plein.hovered?3:mo.retour.hovered?4:-1;
                hover_once(&audio,&last_hover,cur);
                if (ev.type==SDL_KEYDOWN) {
                    if (ev.key.keysym.sym==SDLK_ESCAPE){state_menu=STATE_MAIN;audio_switch(&audio,1);}
                    if (ev.key.keysym.sym==SDLK_PLUS||ev.key.keysym.sym==SDLK_KP_PLUS)
                        {audio.volume=SDL_min(audio.volume+8,MIX_MAX_VOLUME);Mix_VolumeMusic(audio.volume);}
                    if (ev.key.keysym.sym==SDLK_MINUS||ev.key.keysym.sym==SDLK_KP_MINUS)
                        {audio.volume=SDL_max(audio.volume-8,0);Mix_VolumeMusic(audio.volume);}
                }
                if (ev.type==SDL_MOUSEBUTTONDOWN) {
                    if (audio.click_snd) Mix_PlayChannel(-1,audio.click_snd,0);
                    if (mo.diminuer.hovered)  {audio.volume=SDL_max(audio.volume-8,0);Mix_VolumeMusic(audio.volume);}
                    if (mo.augmenter.hovered) {audio.volume=SDL_min(audio.volume+8,MIX_MAX_VOLUME);Mix_VolumeMusic(audio.volume);}
                    if (mo.normal.hovered)    SDL_SetWindowFullscreen(window,0);
                    if (mo.plein.hovered)     SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN_DESKTOP);
                    if (mo.retour.hovered)    {state_menu=STATE_MAIN;audio_switch(&audio,1);}
                }
            }

            /* ── SAUVEGARDE ── */
            else if (state_menu==STATE_SAUVEGARDE) {
                ms.oui.hovered     =(ms.page==0)?btn_hit(&ms.oui,mx,my):0;
                ms.non.hovered     =(ms.page==0)?btn_hit(&ms.non,mx,my):0;
                ms.charger.hovered =(ms.page==1)?btn_hit(&ms.charger,mx,my):0;
                ms.nouvelle.hovered=(ms.page==1)?btn_hit(&ms.nouvelle,mx,my):0;
                ms.retour.hovered  =btn_hit(&ms.retour,mx,my);
                int cur=ms.oui.hovered?0:ms.non.hovered?1:ms.charger.hovered?2:
                        ms.nouvelle.hovered?3:ms.retour.hovered?4:-1;
                hover_once(&audio,&last_hover,cur);
                if (ev.type==SDL_KEYDOWN) {
                    if (ev.key.keysym.sym==SDLK_ESCAPE){state_menu=STATE_MAIN;audio_switch(&audio,1);}
                    if (ev.key.keysym.sym==SDLK_n&&ms.page==1)
                        {state_menu=STATE_JOUEUR;mj.page=0;audio_switch(&audio,4);}
                }
                if (ev.type==SDL_MOUSEBUTTONDOWN) {
                    if (audio.click_snd) Mix_PlayChannel(-1,audio.click_snd,0);
                    if (ms.page==0&&btn_hit(&ms.oui,mx,my)) ms.page=1;
                    if (ms.page==0&&btn_hit(&ms.non,mx,my)){state_menu=STATE_MAIN;audio_switch(&audio,1);}
                    if (btn_hit(&ms.retour,mx,my)){state_menu=STATE_MAIN;audio_switch(&audio,1);}
                    /* Charger jeu */
                    if (ms.page==1&&btn_hit(&ms.charger,mx,my)) {
                        SaveData sd;
                        if (load_save(&sd,save_path)) {
                            GameMode gmode=(GameMode)sd.mode;
                            Mix_HaltMusic();
                            launch_game(renderer,window,font,fsm,&bg,&me,&audio,
                                        gmode,sd.char_id,&sd,save_path,scores_path);
                            audio_switch(&audio,1);
                        }
                        state_menu=STATE_MAIN;
                    }
                    /* Nouvelle partie */
                    if (ms.page==1&&btn_hit(&ms.nouvelle,mx,my))
                        {state_menu=STATE_JOUEUR;mj.page=0;audio_switch(&audio,4);}
                }
            }

            /* ── JOUEUR (choix avatar + mono/multi) ── */
            else if (state_menu==STATE_JOUEUR) {
                mj.mono.hovered   =(mj.page==0)?btn_hit(&mj.mono,mx,my):0;
                mj.multi.hovered  =(mj.page==0)?btn_hit(&mj.multi,mx,my):0;
                mj.avatar1.hovered=(mj.page==1)?btn_hit(&mj.avatar1,mx,my):0;
                mj.avatar2.hovered=(mj.page==1)?btn_hit(&mj.avatar2,mx,my):0;
                mj.valider.hovered=(mj.page==1)?btn_hit(&mj.valider,mx,my):0;
                mj.retour.hovered =btn_hit(&mj.retour,mx,my);
                int cur=mj.mono.hovered?0:mj.multi.hovered?1:mj.avatar1.hovered?2:
                        mj.avatar2.hovered?3:mj.valider.hovered?4:mj.retour.hovered?5:-1;
                hover_once(&audio,&last_hover,cur);
                if (ev.type==SDL_KEYDOWN) {
                    if (ev.key.keysym.sym==SDLK_ESCAPE){state_menu=STATE_MAIN;audio_switch(&audio,1);}
                    if ((ev.key.keysym.sym==SDLK_RETURN||ev.key.keysym.sym==SDLK_KP_ENTER)&&mj.page==1){
                        /* Lancer le jeu */
                        GameMode gmode=(mj.mode==2)?MODE_MULTI:MODE_MONO;
                        int cid=(mj.sel_avatar==1)?0:1;
                        Mix_HaltMusic();
                        launch_game(renderer,window,font,fsm,&bg,&me,&audio,
                                    gmode,cid,NULL,save_path,scores_path);
                        audio_switch(&audio,1);
                        state_menu=STATE_MAIN;
                    }
                }
                if (ev.type==SDL_MOUSEBUTTONDOWN) {
                    if (audio.click_snd) Mix_PlayChannel(-1,audio.click_snd,0);
                    if (mj.page==0) {
                        if (btn_hit(&mj.mono,mx,my))  {mj.mode=1;mj.page=1;}
                        if (btn_hit(&mj.multi,mx,my)) {mj.mode=2;mj.page=1;}
                    } else {
                        if (btn_hit(&mj.avatar1,mx,my)) mj.sel_avatar=1;
                        if (btn_hit(&mj.avatar2,mx,my)) mj.sel_avatar=2;
                        if (btn_hit(&mj.input1,mx,my))  mj.sel_input=1;
                        if (btn_hit(&mj.input2,mx,my))  mj.sel_input=2;
                        if (btn_hit(&mj.valider,mx,my)) {
                            GameMode gmode=(mj.mode==2)?MODE_MULTI:MODE_MONO;
                            int cid=(mj.sel_avatar==1)?0:1;
                            Mix_HaltMusic();
                            launch_game(renderer,window,font,fsm,&bg,&me,&audio,
                                        gmode,cid,NULL,save_path,scores_path);
                            audio_switch(&audio,1);
                            state_menu=STATE_MAIN;
                        }
                    }
                    if (btn_hit(&mj.retour,mx,my)){state_menu=STATE_MAIN;audio_switch(&audio,1);}
                }
            }

            /* ── SCORE ── */
            else if (state_menu==STATE_SCORE) {
                mc.valider.hovered=(mc.page==0)?btn_hit(&mc.valider,mx,my):0;
                mc.retour.hovered =(mc.page==1)?btn_hit(&mc.retour,mx,my):0;
                mc.quitter.hovered=(mc.page==1)?btn_hit(&mc.quitter,mx,my):0;
                int cur=mc.valider.hovered?0:mc.retour.hovered?1:mc.quitter.hovered?2:-1;
                hover_once(&audio,&last_hover,cur);
                if (ev.type==SDL_TEXTINPUT&&mc.page==0) {
                    const char *s=ev.text.text;
                    for (int i=0;s[i]&&mc.nom_len<49;i++) {
                        unsigned char c=(unsigned char)s[i];
                        if (c>=32&&c<127){mc.nom[mc.nom_len++]=(char)c;mc.nom[mc.nom_len]='\0';}
                    }
                    txt_set(renderer,&mc.champ_nom,mc.nom);
                }
                if (ev.type==SDL_KEYDOWN) {
                    SDL_Keycode k2=ev.key.keysym.sym;
                    if (k2==SDLK_ESCAPE){state_menu=STATE_MAIN;audio_switch(&audio,1);mc.music_lancee=0;}
                    if (k2==SDLK_e&&enigme_ok){state_menu=STATE_ENIGME;me.view=0;audio_switch(&audio,0);}
                    if (k2==SDLK_BACKSPACE&&mc.page==0&&mc.nom_len>0)
                        {mc.nom[--mc.nom_len]='\0';txt_set(renderer,&mc.champ_nom,mc.nom);}
                    if ((k2==SDLK_RETURN||k2==SDLK_KP_ENTER)&&mc.page==0&&mc.nom_len>0) {
                        enregistrer_score(&mc,renderer);
                        mc.page=1;mc.music_lancee=0;
                        mc.nom_len=0;mc.nom[0]='\0';txt_set(renderer,&mc.champ_nom,"");
                    }
                }
                if (ev.type==SDL_MOUSEBUTTONDOWN) {
                    if (audio.click_snd) Mix_PlayChannel(-1,audio.click_snd,0);
                    if (mc.page==0&&btn_hit(&mc.valider,mx,my)&&mc.nom_len>0) {
                        enregistrer_score(&mc,renderer);
                        mc.page=1;mc.music_lancee=0;
                        mc.nom_len=0;mc.nom[0]='\0';txt_set(renderer,&mc.champ_nom,"");
                    }
                    if (mc.page==1&&btn_hit(&mc.retour,mx,my)){state_menu=STATE_MAIN;audio_switch(&audio,1);mc.music_lancee=0;mc.page=0;}
                    if (mc.page==1&&btn_hit(&mc.quitter,mx,my)) running=0;
                }
            }

            /* ── ENIGME (depuis menu score) ── */
            else if (state_menu==STATE_ENIGME&&enigme_ok) {
                me.hover_quiz  =(mx>=E_QUIZ_X&&mx<=E_QUIZ_X+E_QW&&my>=E_QUIZ_Y&&my<=E_QUIZ_Y+E_QH);
                me.hover_puzzle=(mx>=E_PUZZLE_X&&mx<=E_PUZZLE_X+E_QW&&my>=E_PUZZLE_Y&&my<=E_PUZZLE_Y+E_QH);
                me.hover_a=(mx>=E_BTN_X&&mx<=E_BTN_X+E_BW&&my>=E_AY&&my<=E_AY+E_BH);
                me.hover_b=(mx>=E_BTN_X&&mx<=E_BTN_X+E_BW&&my>=E_BY&&my<=E_BY+E_BH);
                me.hover_c=(mx>=E_BTN_X&&mx<=E_BTN_X+E_BW&&my>=E_CY&&my<=E_CY+E_BH);
                if (ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE)
                    {state_menu=STATE_SCORE;mc.page=0;mc.music_lancee=0;audio_switch(&audio,2);}
                if (ev.type==SDL_MOUSEBUTTONDOWN) {
                    if (me.view==0) {
                        if (me.hover_quiz)   {me.view=1;audio_switch(&audio,6);enigme_start(&me);}
                        /* PUZZLE → lancer le puzzle */
                        if (me.hover_puzzle) {
                            Mix_HaltMusic();
                            run_puzzle(renderer,window);
                            audio_switch(&audio,0);
                        }
                    } else if (me.fini) {
                        mc.score_joueur=me.score;
                        state_menu=STATE_SCORE;mc.page=0;mc.music_lancee=0;
                        audio_switch(&audio,2);me.view=0;me.fini=0;
                    } else {
                        int ans=me.hover_a?0:me.hover_b?1:me.hover_c?2:-1;
                        if (ans>=0) {
                            Question *q=&me.questions[me.current];
                            if (ans==q->correct){me.score+=10;me.corrects++;}
                            else{me.vies--;me.flash=1;me.flash_start=SDL_GetTicks();}
                            me.total++;me.round_idx++;
                            if (me.vies<=0){me.fini=1;me.show_hearts=0;me.end_anim=SDL_GetTicks();}
                            else if (me.round_idx>=ROUND_SIZE){me.fini=1;me.show_hearts=1;me.end_anim=SDL_GetTicks();}
                            else{me.current=me.round[me.round_idx];me.t_start=SDL_GetTicks();me.temps_ecoule=0;}
                        }
                    }
                }
            }
        } /* fin events */

        /* Update */
        if (state_menu==STATE_ENIGME&&enigme_ok) update_enigme(&me);

        /* Rendu menu */
        SDL_SetRenderDrawColor(renderer,20,20,30,255);
        SDL_RenderClear(renderer);
        switch(state_menu) {
            case STATE_MAIN:       render_principal(renderer,&mp);break;
            case STATE_OPTIONS:    render_options(renderer,&mo);break;
            case STATE_SAUVEGARDE: render_sauvegarde(renderer,&ms);break;
            case STATE_JOUEUR:     render_joueur(renderer,&mj,font_shared);break;
            case STATE_SCORE:      render_score(renderer,&mc,&audio);break;
            case STATE_ENIGME:     if(enigme_ok)render_enigme(renderer,&me);break;
            default:break;
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    /* Cleanup */
    free_principal(&mp); free_options(&mo); free_sauvegarde(&ms);
    free_joueur(&mj);    free_score(&mc);   free_enigme(&me);
    bg_free(&bg);
    if (font_shared) TTF_CloseFont(font_shared);
    if (font)        TTF_CloseFont(font);
    if (fsm)         TTF_CloseFont(fsm);
    if (audio.principal_music)  Mix_FreeMusic(audio.principal_music);
    if (audio.options_music)    Mix_FreeMusic(audio.options_music);
    if (audio.sauvegarde_music) Mix_FreeMusic(audio.sauvegarde_music);
    if (audio.joueur_music)     Mix_FreeMusic(audio.joueur_music);
    if (audio.score_music)      Mix_FreeMusic(audio.score_music);
    if (audio.enigme_music)     Mix_FreeMusic(audio.enigme_music);
    if (audio.hover_snd)        Mix_FreeChunk(audio.hover_snd);
    if (audio.click_snd)        Mix_FreeChunk(audio.click_snd);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}
