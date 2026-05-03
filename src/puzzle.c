#include "../include/puzzle.h"

int initSDL(PuzzleGame *g)
{
    memset(g, 0, sizeof(PuzzleGame));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    if (!(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) & (IMG_INIT_JPG | IMG_INIT_PNG)))
    {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 0;
    }

    g->win = SDL_CreateWindow(
        "Puzzle PuzzleGame",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        0, 0,
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    if (!g->win)
    {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    g->ren = SDL_CreateRenderer(
        g->win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!g->ren)
    {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g->win);
        IMG_Quit();
        SDL_Quit();
        return 0;
    }

    SDL_GetWindowSize(g->win, &g->W, &g->H);

    g->running = 1;
    g->dragging = -1;
    srand((unsigned int)time(NULL));

    return 1;
}

int loadGame(PuzzleGame *g)
{
    FILE *f;
    int len, i;
    char path[128];
    char *bnames[] = {"assets/enigme/btn_a.png", "assets/enigme/btn_b.png", "assets/enigme/btn_c.png"};
    char *hnames[] = {"assets/enigme/btn_a_h.png", "assets/enigme/btn_b_h.png", "assets/enigme/btn_c_h.png"};

  
    g->bg       = IMG_LoadTexture(g->ren, "assets/enigme/background4.png");
    g->img_win  = IMG_LoadTexture(g->ren, "assets/enigme/win.jpeg");
    g->img_over = IMG_LoadTexture(g->ren, "assets/enigme/gameover.jpeg");
    g->img_heart = IMG_LoadTexture(g->ren, "assets/enigme/heart.png");

   

    
    f = fopen("assets/enigme/enigme.txt", "r");
    if (!f)
    {
        fprintf(stderr, "Impossible d'ouvrir assets1/enigme.txt\n");
        return 0;
    }

    g->nbLignes = 0;
    while (g->nbLignes < 20 && fgets(g->lignes[g->nbLignes], 200, f))
    {
        len = (int)strlen(g->lignes[g->nbLignes]);
        while (len > 0 &&
               (g->lignes[g->nbLignes][len - 1] == '\n' ||
                g->lignes[g->nbLignes][len - 1] == '\r'))
        {
            len--;
        }
        g->lignes[g->nbLignes][len] = '\0';

        if (len > 0)
            g->nbLignes++;
    }
    fclose(f);
    

    if (g->nbLignes == 0)
    {
        fprintf(stderr, "enigme.txt est vide\n");
        return 0;
    }

    g->totalPuzzles = (g->nbLignes < MAX_PUZZLES) ? g->nbLignes : MAX_PUZZLES;
    g->current = 0;
    g->waiting = 0;
    g->gameWin = 0;
    g->gameOver = 0;
    g->totalCorrect = 0;
    g->endAngle = 0.0;
    g->endZoom = 0.2;
    g->waitStart = 0;
    g->endAnimStart = 0;

   
   
   
   
    memset(&g->e, 0, sizeof(Enigme));
    sscanf(g->lignes[g->current], "%s %s %s %s %d",
           g->e.question, g->e.rep[0], g->e.rep[1], g->e.rep[2], &g->e.bonne[0]);

    sprintf(path, "assets/enigme/%s", g->e.question);
    g->e.img_question = IMG_LoadTexture(g->ren, path);

    g->e.type = !strcmp(g->e.question, "puzzle3.jpeg") ? 3 : 1;

    if (!strcmp(g->e.question, "puzzle.jpeg"))
        g->e.pos_question = (SDL_Rect){(g->W - 800) / 2, 100, 800, 444};
    else if (!strcmp(g->e.question, "puzzle2.jpeg"))
        g->e.pos_question = (SDL_Rect){(g->W - 640) / 2, 100, 640, 400};
    else
        g->e.pos_question = (SDL_Rect){(g->W - 800) / 2, 100, 800, 450};

    if (g->e.type == 1)
    {
        int hx = !strcmp(g->e.question, "puzzle.jpeg") ? 145 : 290;
        int hy = !strcmp(g->e.question, "puzzle.jpeg") ? 232 : 23;
        g->e.snap[0] = (SDL_Rect){g->e.pos_question.x + hx, g->e.pos_question.y + hy, 150, 150};
        g->e.bonne[0]--;
    }
    else
    {
        float sx = g->e.pos_question.w / 800.0f;
        float sy = g->e.pos_question.h / 450.0f;
        g->e.snap[0] = (SDL_Rect){g->e.pos_question.x + (int)(113 * sx), g->e.pos_question.y + (int)(147 * sy), 150, 150};
        g->e.snap[1] = (SDL_Rect){g->e.pos_question.x + (int)(518 * sx), g->e.pos_question.y + (int)(259 * sy), 150, 150};
        g->e.snap[2] = (SDL_Rect){g->e.pos_question.x + (int)(603 * sx), g->e.pos_question.y + (int)(22  * sy), 150, 150};
        g->e.bonne[0] = 0;
        g->e.bonne[1] = 1;
        g->e.bonne[2] = 2;
    }

    for (i = 0; i < N; i++)
    {
        sprintf(path, "assets/enigme/%s", g->e.rep[i]);
        g->e.img_rep[i] = IMG_LoadTexture(g->ren, path);
        g->e.btn[i] = IMG_LoadTexture(g->ren, bnames[i]);
        g->e.btn_hover[i] = IMG_LoadTexture(g->ren, hnames[i]);
    }

    g->e.img_correct = IMG_LoadTexture(g->ren, "assets/enigme/correct.png");
    g->e.img_wrong   = IMG_LoadTexture(g->ren, "assets/enigme/wrong.png");

    {
        int baseY = g->e.pos_question.y + g->e.pos_question.h + 20;
        for (i = 0; i < N; i++)
        {
            g->e.pos_rep[i]  = (SDL_Rect){g->W / 2 - 250 + i * 180, baseY, 150, 150};
            g->e.pos_init[i] = g->e.pos_rep[i];
            g->e.placed[i]   = -1;
            g->e.pos_btn[i]  = (SDL_Rect){g->e.pos_init[i].x + 15, g->e.pos_init[i].y + 175, 120, 40};
        }
    }

    g->e.startTime  = SDL_GetTicks();
    g->e.animStart  = 0;
    g->e.timeBar    = (SDL_Rect){50, 30, g->W - 100, 25};
    g->e.timeLeft   = TIMER_MAX;
    g->e.etat       = 0;
    g->e.showResult = 0;
    g->e.angle      = 0.0;
    g->e.zoom       = 0.2;

    return 1;
}

void handleInput(PuzzleGame *g)
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_QUIT)
        {
            g->running = 0;
            return;
        }

        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
        {
            g->running = 0;
            return;
        }

        if (g->gameWin || g->gameOver || g->waiting || g->e.etat)
            continue;

        
        
        
        if (g->e.showResult == 2)
            continue;

        if (ev.type == SDL_MOUSEBUTTONDOWN)
        {
            int i, z;
            SDL_Point p = {ev.button.x, ev.button.y};

            for (i = 0; i < N; i++)
            {
                if (SDL_PointInRect(&p, &g->e.pos_rep[i]))
                {
                    g->dragging = i;

                    
                    
                    
                    for (z = 0; z < N; z++)
                    {
                        if (g->e.placed[z] == i)
                        {
                            g->e.placed[z] = -1;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if (ev.type == SDL_MOUSEMOTION && g->dragging != -1)
        {
            g->e.pos_rep[g->dragging].x = ev.motion.x - 75;
            g->e.pos_rep[g->dragging].y = ev.motion.y - 75;
        }

        if (ev.type == SDL_MOUSEBUTTONUP && g->dragging != -1)
        {
            int zones = (g->e.type == 1) ? 1 : N;
            int z = -1;
            int i;

            for (i = 0; i < zones; i++)
            {
                if (SDL_HasIntersection(&g->e.pos_rep[g->dragging], &g->e.snap[i]))
                    z = i;
            }

            if (z != -1)
            {
                
                if (g->e.placed[z] != -1 && g->e.placed[z] != g->dragging)
                    g->e.pos_rep[g->e.placed[z]] = g->e.pos_init[g->e.placed[z]];

                
                g->e.pos_rep[g->dragging].x = g->e.snap[z].x;
                g->e.pos_rep[g->dragging].y = g->e.snap[z].y;
                g->e.placed[z] = g->dragging;

                
                
                if (g->e.placed[z] == g->e.bonne[z])
                {
                    g->e.showResult = 1;
                    g->e.animStart = SDL_GetTicks();
                    g->e.angle = 0;
                    g->e.zoom = 0.2;

                    if (g->e.type == 1)
                    {
                        g->e.etat = 1;
                    }
                    else
                    {
                        int okAll = 1;
                        for (i = 0; i < N; i++)
                        {
                            if (g->e.placed[i] != g->e.bonne[i])
                            {
                                okAll = 0;
                                break;
                            }
                        }
                        if (okAll) g->e.etat = 1;
                    }
                }
                else
                {
                    g->e.showResult = 2;
                    g->e.animStart = SDL_GetTicks();
                    g->e.angle = 0;
                    g->e.zoom = 0.2;
                }
            }
            else
            {
                
           
           
                g->e.pos_rep[g->dragging] = g->e.pos_init[g->dragging];
            }

            g->dragging = -1;
        }
    }
}

void updateGame(PuzzleGame *g)
{
    int i;
    int elapsed;
    char path[128];
    char *bnames[] = {"assets/enigme/btn_a.png", "assets/enigme/btn_b.png", "assets/enigme/btn_c.png"};
    char *hnames[] = {"assets/enigme/btn_a_h.png", "assets/enigme/btn_b_h.png", "assets/enigme/btn_c_h.png"};

    if (g->gameWin || g->gameOver)
        return;

     
     
    if (!g->e.etat)
    {
        elapsed = (int)((SDL_GetTicks() - g->e.startTime) / 1000.0f);
        g->e.timeLeft = TIMER_MAX - elapsed;
        if (g->e.timeLeft < 0) g->e.timeLeft = 0;
    }

    
    
    if (!g->waiting && !g->e.etat && g->e.timeLeft <= 0)
    {
        cleanupGame(&(PuzzleGame){
            .e = g->e
        }); 
    }

    
    if (g->e.showResult && SDL_GetTicks() - g->e.animStart > ANIM_MS)
        g->e.showResult = 0;

    
    if (g->e.etat && !g->waiting)
    {
        g->waiting = 1;
        g->waitStart = SDL_GetTicks();
        g->totalCorrect++;
    }

    
    if ((!g->waiting && !g->e.etat && g->e.timeLeft <= 0) ||
        (g->waiting && SDL_GetTicks() - g->waitStart > ANIM_MS + 200))
    {
        
        
        if (g->e.img_question) SDL_DestroyTexture(g->e.img_question);
        if (g->e.img_correct)  SDL_DestroyTexture(g->e.img_correct);
        if (g->e.img_wrong)    SDL_DestroyTexture(g->e.img_wrong);
        for (i = 0; i < N; i++)
        {
            if (g->e.img_rep[i])   SDL_DestroyTexture(g->e.img_rep[i]);
            if (g->e.btn[i])       SDL_DestroyTexture(g->e.btn[i]);
            if (g->e.btn_hover[i]) SDL_DestroyTexture(g->e.btn_hover[i]);
        }

        memset(&g->e, 0, sizeof(Enigme));
        g->waiting = 0;
        g->current++;

        if (g->current >= g->totalPuzzles)
        {
            if (g->totalCorrect >= 1) g->gameWin = 1;
            else                      g->gameOver = 1;

            g->endAnimStart = SDL_GetTicks();
            g->endAngle = 0.0;
            g->endZoom = 0.2;
            return;
        }

       
       
        sscanf(g->lignes[g->current], "%s %s %s %s %d",
               g->e.question, g->e.rep[0], g->e.rep[1], g->e.rep[2], &g->e.bonne[0]);

        sprintf(path, "assets/enigme/%s", g->e.question);
        g->e.img_question = IMG_LoadTexture(g->ren, path);

        g->e.type = !strcmp(g->e.question, "puzzle3.jpeg") ? 3 : 1;

        if (!strcmp(g->e.question, "puzzle.jpeg"))
            g->e.pos_question = (SDL_Rect){(g->W - 800) / 2, 100, 800, 444};
        else if (!strcmp(g->e.question, "puzzle2.jpeg"))
            g->e.pos_question = (SDL_Rect){(g->W - 640) / 2, 100, 640, 400};
        else
            g->e.pos_question = (SDL_Rect){(g->W - 800) / 2, 100, 800, 450};

        if (g->e.type == 1)
        {
            int hx = !strcmp(g->e.question, "puzzle.jpeg") ? 145 : 290;
            int hy = !strcmp(g->e.question, "puzzle.jpeg") ? 232 : 23;
            g->e.snap[0] = (SDL_Rect){g->e.pos_question.x + hx, g->e.pos_question.y + hy, 150, 150};
            g->e.bonne[0]--;
        }
        else
        {
            float sx = g->e.pos_question.w / 800.0f;
            float sy = g->e.pos_question.h / 450.0f;
            g->e.snap[0] = (SDL_Rect){g->e.pos_question.x + (int)(113 * sx), g->e.pos_question.y + (int)(147 * sy), 150, 150};
            g->e.snap[1] = (SDL_Rect){g->e.pos_question.x + (int)(518 * sx), g->e.pos_question.y + (int)(259 * sy), 150, 150};
            g->e.snap[2] = (SDL_Rect){g->e.pos_question.x + (int)(603 * sx), g->e.pos_question.y + (int)(22  * sy), 150, 150};
            g->e.bonne[0] = 0;
            g->e.bonne[1] = 1;
            g->e.bonne[2] = 2;
        }

        for (i = 0; i < N; i++)
        {
            sprintf(path, "assets/enigme/%s", g->e.rep[i]);
            g->e.img_rep[i] = IMG_LoadTexture(g->ren, path);
            g->e.btn[i] = IMG_LoadTexture(g->ren, bnames[i]);
            g->e.btn_hover[i] = IMG_LoadTexture(g->ren, hnames[i]);
        }

        g->e.img_correct = IMG_LoadTexture(g->ren, "assets/enigme/correct.png");
        g->e.img_wrong   = IMG_LoadTexture(g->ren, "assets/enigme/wrong.png");

        {
            int baseY = g->e.pos_question.y + g->e.pos_question.h + 20;
            for (i = 0; i < N; i++)
            {
                g->e.pos_rep[i]  = (SDL_Rect){g->W / 2 - 250 + i * 180, baseY, 150, 150};
                g->e.pos_init[i] = g->e.pos_rep[i];
                g->e.placed[i]   = -1;
                g->e.pos_btn[i]  = (SDL_Rect){g->e.pos_init[i].x + 15, g->e.pos_init[i].y + 175, 120, 40};
            }
        }

        g->e.startTime  = SDL_GetTicks();
        g->e.animStart  = 0;
        g->e.timeBar    = (SDL_Rect){50, 30, g->W - 100, 25};
        g->e.timeLeft   = TIMER_MAX;
        g->e.etat       = 0;
        g->e.showResult = 0;
        g->e.angle      = 0.0;
        g->e.zoom       = 0.2;
    }
}

void renderGame(PuzzleGame *g)
{
    int i, mx, my, iw, ih, zones, tw;
    SDL_Rect dst, bar;
    SDL_Point mp;

    SDL_RenderClear(g->ren);
    if (g->bg) SDL_RenderCopy(g->ren, g->bg, NULL, NULL);

    
    if (g->gameWin || g->gameOver)
    {
        SDL_Texture *endImg = g->gameWin ? g->img_win : g->img_over;

        if (endImg)
        {
            g->endAngle += 4.0;
            if (g->endZoom < 1.0) g->endZoom += 0.03;

            SDL_QueryTexture(endImg, NULL, NULL, &iw, &ih);
            dst.x = g->W / 2 - (int)(iw * g->endZoom) / 2;
            dst.y = g->H / 2 - (int)(ih * g->endZoom) / 2;
            dst.w = (int)(iw * g->endZoom);
            dst.h = (int)(ih * g->endZoom);

            SDL_RenderCopyEx(g->ren, endImg, NULL, &dst, g->endAngle, NULL, SDL_FLIP_NONE);
        }

        
        
        if (g->gameWin && g->img_heart)
        {
            int size = g->W / 20;
            if (size < 35) size = 35;
            if (size > 70) size = 70;

            for (i = 0; i < g->totalCorrect; i++)
            {
                SDL_Rect heartDst = {
                    g->W - (i + 1) * (size + 10) - 20,
                    20,
                    size,
                    size
                };
                SDL_RenderCopy(g->ren, g->img_heart, NULL, &heartDst);
            }
        }

        SDL_RenderPresent(g->ren);
        return;
    }

    
    mx = 0;
    my = 0;
    SDL_GetMouseState(&mx, &my);

    if (g->e.img_question)
        SDL_RenderCopy(g->ren, g->e.img_question, NULL, &g->e.pos_question);

    SDL_SetRenderDrawColor(g->ren, 0, 0, 180, 255);
    for (i = 0; i < N; i++)
    {
        int t;
        for (t = 0; t < 4; t++)
        {
            SDL_Rect rr = {
                g->e.pos_init[i].x - t,
                g->e.pos_init[i].y - t,
                g->e.pos_init[i].w + 2 * t,
                g->e.pos_init[i].h + 2 * t
            };
            SDL_RenderDrawRect(g->ren, &rr);
        }
    }

    for (i = 0; i < N; i++)
        if (g->e.img_rep[i])
            SDL_RenderCopy(g->ren, g->e.img_rep[i], NULL, &g->e.pos_rep[i]);

    for (i = 0; i < N; i++)
    {
        mp.x = mx;
        mp.y = my;
        if (SDL_PointInRect(&mp, &g->e.pos_btn[i]) && g->e.btn_hover[i])
            SDL_RenderCopy(g->ren, g->e.btn_hover[i], NULL, &g->e.pos_btn[i]);
        else if (g->e.btn[i])
            SDL_RenderCopy(g->ren, g->e.btn[i], NULL, &g->e.pos_btn[i]);
    }

    zones = (g->e.type == 1) ? 1 : N;
    for (i = 0; i < zones; i++)
    {
        if      (g->e.placed[i] == -1)          SDL_SetRenderDrawColor(g->ren, 0, 120, 255, 255);
        else if (g->e.placed[i] == g->e.bonne[i]) SDL_SetRenderDrawColor(g->ren, 0, 255, 0, 255);
        else                                    SDL_SetRenderDrawColor(g->ren, 255, 0, 0, 255);

        {
            int t;
            for (t = 0; t < 3; t++)
            {
                SDL_Rect rr = {
                    g->e.snap[i].x - t,
                    g->e.snap[i].y - t,
                    g->e.snap[i].w + 2 * t,
                    g->e.snap[i].h + 2 * t
                };
                SDL_RenderDrawRect(g->ren, &rr);
            }
        }
    }

    
    tw = g->e.timeLeft * g->e.timeBar.w / TIMER_MAX;
    bar = (SDL_Rect){g->e.timeBar.x, g->e.timeBar.y, tw, g->e.timeBar.h};

    if      (g->e.timeLeft > 9) SDL_SetRenderDrawColor(g->ren, 0, 255, 0, 255);
    else if (g->e.timeLeft > 5) SDL_SetRenderDrawColor(g->ren, 255, 165, 0, 255);
    else                        SDL_SetRenderDrawColor(g->ren, 255, 0, 0, 255);

    SDL_RenderFillRect(g->ren, &bar);

   
    if (g->img_heart)
    {
        int size = g->W / 20;
        if (size < 35) size = 35;
        if (size > 70) size = 70;

        for (i = 0; i < g->totalCorrect; i++)
        {
            SDL_Rect heartDst = {
                g->W - (i + 1) * (size + 10) - 20,
                20,
                size,
                size
            };
            SDL_RenderCopy(g->ren, g->img_heart, NULL, &heartDst);
        }
    }

    
    if (g->e.showResult)
    {
        SDL_Texture *img = (g->e.showResult == 1) ? g->e.img_correct : g->e.img_wrong;
        if (img)
        {
            g->e.angle += 4.0;
            if (g->e.zoom < 1.0) g->e.zoom += 0.03;

            SDL_QueryTexture(img, NULL, NULL, &iw, &ih);
            dst.x = g->W / 2 - (int)(iw * g->e.zoom) / 2;
            dst.y = g->H / 2 - (int)(ih * g->e.zoom) / 2;
            dst.w = (int)(iw * g->e.zoom);
            dst.h = (int)(ih * g->e.zoom);

            SDL_RenderCopyEx(g->ren, img, NULL, &dst, g->e.angle, NULL, SDL_FLIP_NONE);
        }
    }

    SDL_RenderPresent(g->ren);
}

void cleanupGame(PuzzleGame *g)
{
    int i;

    if (g->e.img_question) SDL_DestroyTexture(g->e.img_question);
    if (g->e.img_correct)  SDL_DestroyTexture(g->e.img_correct);
    if (g->e.img_wrong)    SDL_DestroyTexture(g->e.img_wrong);

    for (i = 0; i < N; i++)
    {
        if (g->e.img_rep[i])   SDL_DestroyTexture(g->e.img_rep[i]);
        if (g->e.btn[i])       SDL_DestroyTexture(g->e.btn[i]);
        if (g->e.btn_hover[i]) SDL_DestroyTexture(g->e.btn_hover[i]);
    }

    if (g->bg)        SDL_DestroyTexture(g->bg);
    if (g->img_win)   SDL_DestroyTexture(g->img_win);
    if (g->img_over)  SDL_DestroyTexture(g->img_over);
    if (g->img_heart) SDL_DestroyTexture(g->img_heart);

    if (g->ren) SDL_DestroyRenderer(g->ren);
    if (g->win) SDL_DestroyWindow(g->win);

    IMG_Quit();
    SDL_Quit();
}
