#include "../include/jeu_entite.h"

SDL_Texture* entite_load_tex(const char *path, SDL_Renderer *r)
{
    SDL_Surface *s = IMG_Load(path);
    if (!s) { SDL_Log("entite_load(%s): %s", path, IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    return t;
}

void hearts_init(Heart *hearts, int nb, int *xs, int *ys)
{
    for (int i = 0; i < nb; i++) {
        hearts[i].pos       = (SDL_Rect){xs[i], ys[i], 55, 55};
        hearts[i].collected = 0;
    }
}

void bows_init(Bow *bows, int nb, int *xs, int *ys)
{
    for (int i = 0; i < nb; i++) {
        bows[i].pos       = (SDL_Rect){xs[i], ys[i], 600, 600};
        bows[i].collected = 0;
    }
}

/* collision boite englobante */
static int bb(SDL_Rect a, SDL_Rect b)
{
    return !(a.x+a.w < b.x || a.x > b.x+b.w ||
             a.y+a.h < b.y || a.y > b.y+b.h);
}

int heart_check(Heart *h, SDL_Rect player)
{
    if (h->collected) return 0;
    if (bb(h->pos, player)) { h->collected = 1; return 1; }
    return 0;
}

int bow_check(Bow *b, SDL_Rect player)
{
    if (b->collected) return 0;
    if (bb(b->pos, player)) { b->collected = 1; return 1; }
    return 0;
}

/* Rendu avec clip à la zone de vue (split screen) */
static void render_item(SDL_Renderer *r, SDL_Rect world_pos,
                         SDL_Texture *tex, int collected,
                         int cam_x, int cam_y,
                         int view_x, int view_w)
{
    if (collected || !tex) return;
    int sx = (world_pos.x - cam_x) + view_x;
    int sy = (world_pos.y - cam_y);
    if (sx + world_pos.w < view_x || sx > view_x + view_w) return;

    SDL_Rect clip = {view_x, 0, view_w, 720};
    SDL_RenderSetClipRect(r, &clip);
    SDL_Rect dst = {sx, sy, world_pos.w, world_pos.h};
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_RenderSetClipRect(r, NULL);
}

void hearts_render(SDL_Renderer *r, Heart *hearts, int nb,
                   SDL_Texture *tex, int cam_x, int cam_y,
                   int view_x, int view_w)
{
    for (int i = 0; i < nb; i++)
        render_item(r, hearts[i].pos, tex, hearts[i].collected,
                    cam_x, cam_y, view_x, view_w);
}

void bows_render(SDL_Renderer *r, Bow *bows, int nb,
                 SDL_Texture *tex, int cam_x, int cam_y,
                 int view_x, int view_w)
{
    for (int i = 0; i < nb; i++)
        render_item(r, bows[i].pos, tex, bows[i].collected,
                    cam_x, cam_y, view_x, view_w);
}
