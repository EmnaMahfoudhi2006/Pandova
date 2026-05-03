#ifndef PUZZLE_H
#define PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define N 3
#define TIMER_MAX 15
#define ANIM_MS 2000
#define MAX_PUZZLES 3

typedef struct
{
    int type;                       
    char question[50];
    char rep[N][50];

    int bonne[N];                   
    int placed[N];                  
    int timeLeft;
    int etat;                       
    int showResult;             

    double angle, zoom;
    Uint32 startTime, animStart;

    SDL_Rect pos_question;
    SDL_Rect pos_rep[N];
    SDL_Rect pos_init[N];
    SDL_Rect pos_btn[N];
    SDL_Rect snap[N];
    SDL_Rect timeBar;

    SDL_Texture *img_question;
    SDL_Texture *img_rep[N];
    SDL_Texture *btn[N];
    SDL_Texture *btn_hover[N];
    SDL_Texture *img_correct;
    SDL_Texture *img_wrong;
} Enigme;

typedef struct
{
    SDL_Window *win;
    SDL_Renderer *ren;

    int W, H;
    int running;

    SDL_Texture *bg;
    SDL_Texture *img_win;
    SDL_Texture *img_over;
    SDL_Texture *img_heart;

    char lignes[20][200];
    int nbLignes;

    int totalPuzzles;
    int current;
    int dragging;
    int waiting;

    int gameWin;
    int gameOver;
    int totalCorrect;

    double endAngle;
    double endZoom;
    Uint32 waitStart;
    Uint32 endAnimStart;

    Enigme e;
} PuzzleGame;


int initSDL(PuzzleGame *g);


int loadGame(PuzzleGame *g);


void handleInput(PuzzleGame *g);


void updateGame(PuzzleGame *g);


void renderGame(PuzzleGame *g);


void cleanupGame(PuzzleGame *g);

#endif
