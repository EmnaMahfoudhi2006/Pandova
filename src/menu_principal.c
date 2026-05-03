#include "../include/menu_principal.h"

void init_principal(SDL_Renderer *r, MenuPrincipal *m)
{
    m->bg   = tex_load(r, P_PRINCIPAL "BG.png");
    m->logo = tex_load(r, P_PRINCIPAL "logo.png");

    int bw=220, bh=72, bx=60, by=170, gap=100;
    btn_init(r,&m->jouer,    P_PRINCIPAL"JOUER.png",           P_PRINCIPAL"JOUER_CLICK.png",           bx, by+0*gap, bw, bh);
    btn_init(r,&m->options,  P_PRINCIPAL"OPTIONS.png",         P_PRINCIPAL"OPTIONS_CLICK.png",         bx, by+1*gap, bw, bh);
    btn_init(r,&m->score,    P_PRINCIPAL"MEILLEUR SCORES.png", P_PRINCIPAL"MEILLEUR_SCORES_CLICK.png", bx, by+2*gap, bw, bh);
    btn_init(r,&m->histoire, P_PRINCIPAL"HISTOIRE.png",        P_PRINCIPAL"HISTOIRE_CLICK.png",        bx, by+3*gap, bw, bh);
    btn_init(r,&m->quitter,  P_PRINCIPAL"QUITTER.png",         P_PRINCIPAL"QUITTER_CLICK.png",         WIN_W-250, WIN_H-100, 220, 72);
}

void render_principal(SDL_Renderer *r, MenuPrincipal *m)
{
    if (m->bg) SDL_RenderCopy(r, m->bg, NULL, NULL);
    if (m->logo) {
        SDL_Rect lr = {WIN_W-220, WIN_H-190, 200, 170};
        SDL_RenderCopy(r, m->logo, NULL, &lr);
    }
    btn_render(r, &m->jouer);
    btn_render(r, &m->options);
    btn_render(r, &m->score);
    btn_render(r, &m->histoire);
    btn_render(r, &m->quitter);
}

void free_principal(MenuPrincipal *m)
{
    if (m->bg)   SDL_DestroyTexture(m->bg);
    if (m->logo) SDL_DestroyTexture(m->logo);
    btn_free(&m->jouer);   btn_free(&m->options);
    btn_free(&m->score);   btn_free(&m->histoire);
    btn_free(&m->quitter);
}
