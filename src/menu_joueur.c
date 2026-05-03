#include "../include/menu_joueur.h"

void init_joueur(SDL_Renderer *r, MenuJoueur *m)
{
    m->bg = tex_load(r, P_JOUEUR "bg.png");

    int cx = WIN_W/2, cy = WIN_H/2;
    btn_init(r, &m->mono,    P_JOUEUR"mono.jpeg",    P_JOUEUR"monoH.jpeg",    cx-230, cy-150, 200, 80);
    btn_init(r, &m->multi,   P_JOUEUR"multi.jpeg",   P_JOUEUR"multiH.jpeg",   cx+30,  cy-150, 200, 80);
    btn_init(r, &m->avatar1, P_JOUEUR"neytiry.jpeg", P_JOUEUR"neytiryH.jpeg", cx-180, cy-70,  150, 70);
    btn_init(r, &m->avatar2, P_JOUEUR"jake.jpeg",    P_JOUEUR"jakeH.jpeg",    cx+30,  cy-70,  150, 70);
    btn_init(r, &m->input1,  P_JOUEUR"input2.jpeg",  P_JOUEUR"input2H.jpeg",  cx-180, cy+20,  150, 70);
    btn_init(r, &m->input2,  P_JOUEUR"input2.jpeg",  P_JOUEUR"input2H.jpeg",  cx+30,  cy+20,  150, 70);
    btn_init(r, &m->valider, P_JOUEUR"valider.jpeg", P_JOUEUR"validerH.jpeg", cx-80,  cy+130, 160, 70);
    btn_init(r, &m->retour,  P_JOUEUR"retour.jpeg",  P_JOUEUR"retourH.jpeg",  WIN_W-170, WIN_H-90, 150, 65);

    m->page = 0; m->mode = 0; m->sel_avatar = 1; m->sel_input = 1;
}

void render_joueur(SDL_Renderer *r, MenuJoueur *m, TTF_Font *f)
{
    if (m->bg) SDL_RenderCopy(r, m->bg, NULL, NULL);

    if (m->page == 0) {
        btn_render(r, &m->mono);
        btn_render(r, &m->multi);
    } else {
        /* Cadre jaune autour du bouton sélectionné */
        SDL_SetRenderDrawColor(r, 255, 200, 0, 255);
        Btn *sel_av = (m->sel_avatar == 1) ? &m->avatar1 : &m->avatar2;
        SDL_Rect fa = {sel_av->dst.x-4, sel_av->dst.y-4, sel_av->dst.w+8, sel_av->dst.h+8};
        SDL_RenderDrawRect(r, &fa);
        Btn *sel_in = (m->sel_input  == 1) ? &m->input1  : &m->input2;
        SDL_Rect fi = {sel_in->dst.x-4, sel_in->dst.y-4, sel_in->dst.w+8, sel_in->dst.h+8};
        SDL_RenderDrawRect(r, &fi);

        btn_render(r, &m->avatar1); btn_render(r, &m->avatar2);
        btn_render(r, &m->input1);  btn_render(r, &m->input2);
        btn_render(r, &m->valider);

        if (f) {
            SDL_Color w = {255, 255, 255, 255};
            draw_text(r, f, "Avatar :", m->avatar1.dst.x-90, m->avatar1.dst.y+20, w);
            draw_text(r, f, "Input :",  m->input1.dst.x-80,  m->input1.dst.y+20,  w);
        }
    }
    btn_render(r, &m->retour);
}

void free_joueur(MenuJoueur *m)
{
    if (m->bg) SDL_DestroyTexture(m->bg);
    btn_free(&m->mono);    btn_free(&m->multi);
    btn_free(&m->avatar1); btn_free(&m->avatar2);
    btn_free(&m->input1);  btn_free(&m->input2);
    btn_free(&m->valider); btn_free(&m->retour);
}
