#include "../include/menu_sauvegarde.h"

void init_sauvegarde(SDL_Renderer *r, MenuSauvegarde *m)
{
    m->bg = tex_load(r, P_SAUVEGARDE "backg2.jpg");

    btn_init(r, &m->oui,      P_SAUVEGARDE"oui.png",            P_SAUVEGARDE"oui_hover.png",      WIN_W/2-230, 450, 200, 80);
    btn_init(r, &m->non,      P_SAUVEGARDE"non.png",            P_SAUVEGARDE"non_hover.png",      WIN_W/2+30,  450, 200, 80);
    btn_init(r, &m->charger,  P_SAUVEGARDE"charger.png",        P_SAUVEGARDE"charger_hover.png",  WIN_W/2-140, 260, 280, 90);
    btn_init(r, &m->nouvelle, P_SAUVEGARDE"nouvelle.png",       P_SAUVEGARDE"nouvelle_hover.png", WIN_W/2-140, 400, 280, 90);
    btn_init(r, &m->retour,   P_SAUVEGARDE"retour.png",         P_SAUVEGARDE"retour_hover.png",   WIN_W-200, WIN_H-90, 170, 65);

    txt_init(r, &m->question, 200, 280, 255, 255, 255,
             P_SAUVEGARDE"arial.ttf", 36,
             "Voulez-vous sauvegarder votre jeu ?");
    m->question.rect.x = WIN_W/2 - m->question.rect.w/2;
    m->page = 0;
}

void render_sauvegarde(SDL_Renderer *r, MenuSauvegarde *m)
{
    if (m->bg) SDL_RenderCopy(r, m->bg, NULL, NULL);

    if (m->page == 0) {
        txt_render(r, &m->question);
        btn_render(r, &m->oui);
        btn_render(r, &m->non);
    } else {
        btn_render(r, &m->charger);
        btn_render(r, &m->nouvelle);
    }
    btn_render(r, &m->retour);
}

void free_sauvegarde(MenuSauvegarde *m)
{
    if (m->bg) SDL_DestroyTexture(m->bg);
    btn_free(&m->oui);     btn_free(&m->non);
    btn_free(&m->charger); btn_free(&m->nouvelle);
    btn_free(&m->retour);
    txt_free(&m->question);
}
