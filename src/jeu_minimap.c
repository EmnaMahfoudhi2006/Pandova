#include "../include/jeu_minimap.h"

void minimap_render(SDL_Renderer *r, Background *bg,
                    int mm_x, int mm_y,
                    int p1_wx, int p1_wy,
                    int p2_wx, int p2_wy,
                    int *ex, int *ey, int nb_e,
                    int *bx, int *by, int *bc, int nb_b)
{
    Level *lv = &bg->levels[bg->cur];
    int ww = lv->width;
    int wh = lv->height;

    /* fond semi-transparent */
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 170);
    SDL_Rect frame = {mm_x-2, mm_y-2, MM_W+4, MM_H+4};
    SDL_RenderFillRect(r, &frame);

    /* background miniature */
    if (lv->tex) {
        SDL_Rect src = {0, 0, ww, wh};
        SDL_Rect dst = {mm_x, mm_y, MM_W, MM_H};
        SDL_RenderCopy(r, lv->tex, &src, &dst);
    }

    float sx = (float)MM_W / ww;
    float sy = (float)MM_H / wh;

    /* joueur 1 : point blanc */
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_Rect p1m = {mm_x + (int)(p1_wx*sx)-3, mm_y + (int)(p1_wy*sy)-3, 7, 7};
    SDL_RenderFillRect(r, &p1m);

    /* joueur 2 : point cyan */
    SDL_SetRenderDrawColor(r, 100, 220, 255, 255);
    SDL_Rect p2m = {mm_x + (int)(p2_wx*sx)-3, mm_y + (int)(p2_wy*sy)-3, 7, 7};
    SDL_RenderFillRect(r, &p2m);

    /* ennemis : rouge */
    SDL_SetRenderDrawColor(r, 220, 40, 40, 255);
    for (int i = 0; i < nb_e; i++) {
        SDL_Rect em = {mm_x+(int)(ex[i]*sx)-2, mm_y+(int)(ey[i]*sy)-2, 5, 5};
        SDL_RenderFillRect(r, &em);
    }

    /* bows non collectés : jaune */
    SDL_SetRenderDrawColor(r, 255, 220, 0, 255);
    for (int i = 0; i < nb_b; i++) {
        if (bc && bc[i]) continue;
        SDL_Rect bm = {mm_x+(int)(bx[i]*sx)-2, mm_y+(int)(by[i]*sy)-2, 4, 4};
        SDL_RenderFillRect(r, &bm);
    }

    /* bordure */
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_RenderDrawRect(r, &frame);
}
