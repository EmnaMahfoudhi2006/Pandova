#include "../include/menu_options.h"

void init_options(SDL_Renderer *r, MenuOptions *m)
{
    m->bg = tex_load(r, P_OPTIONS "bg.jpeg");
    btn_init(r, &m->diminuer,  P_OPTIONS"img1.png", P_OPTIONS"img1v.png", 300, 100, 200, 60);
    btn_init(r, &m->augmenter, P_OPTIONS"img2.png", P_OPTIONS"img2v.png", 300, 180, 200, 60);
    btn_init(r, &m->normal,    P_OPTIONS"img3.png", P_OPTIONS"img3v.png", 300, 260, 200, 60);
    btn_init(r, &m->plein,     P_OPTIONS"img4.png", P_OPTIONS"img4v.png", 300, 340, 200, 60);
    btn_init(r, &m->retour,    P_OPTIONS"img5.png", P_OPTIONS"img5v.png", 300, 420, 200, 60);
}

void render_options(SDL_Renderer *r, MenuOptions *m)
{
    if (m->bg) SDL_RenderCopy(r, m->bg, NULL, NULL);
    btn_render(r, &m->diminuer);
    btn_render(r, &m->augmenter);
    btn_render(r, &m->normal);
    btn_render(r, &m->plein);
    btn_render(r, &m->retour);
}

void free_options(MenuOptions *m)
{
    if (m->bg) SDL_DestroyTexture(m->bg);
    btn_free(&m->diminuer); btn_free(&m->augmenter);
    btn_free(&m->normal);   btn_free(&m->plein);
    btn_free(&m->retour);
}
