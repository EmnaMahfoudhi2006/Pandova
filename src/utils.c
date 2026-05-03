#include "../include/header.h"

/* ==================== TEXTURE ==================== */
SDL_Texture* tex_load(SDL_Renderer *r, const char *path)
{
    SDL_Surface *s = IMG_Load(path);
    if (!s) { SDL_Log("IMG_Load(%s): %s", path, IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    if (!t) SDL_Log("CreateTex(%s): %s", path, SDL_GetError());
    return t;
}

/* ==================== BOUTON ==================== */
void btn_init(SDL_Renderer *r, Btn *b,
              const char *p, const char *ph,
              int x, int y, int w, int h)
{
    memset(b, 0, sizeof(*b));
    b->tex   = tex_load(r, p);
    b->tex_h = tex_load(r, ph);
    if (!b->tex_h) b->tex_h = b->tex;
    b->dst = (SDL_Rect){x, y, w, h};
}

void btn_render(SDL_Renderer *r, Btn *b)
{
    SDL_Texture *t = (b->hovered && b->tex_h) ? b->tex_h : b->tex;
    if (t) SDL_RenderCopy(r, t, NULL, &b->dst);
}

int btn_hit(Btn *b, int x, int y)
{
    return (x >= b->dst.x && x <= b->dst.x + b->dst.w &&
            y >= b->dst.y && y <= b->dst.y + b->dst.h);
}

void btn_free(Btn *b)
{
    if (!b) return;
    if (b->tex && b->tex != b->tex_h) SDL_DestroyTexture(b->tex);
    if (b->tex_h) SDL_DestroyTexture(b->tex_h);
    b->tex = b->tex_h = NULL;
}

/* ==================== TEXTE ==================== */
static void txt_rerender(SDL_Renderer *r, TxtLabel *t)
{
    if (t->texture) { SDL_DestroyTexture(t->texture); t->texture = NULL; }
    if (!t->font) return;
    const char *s = t->txt[0] ? t->txt : " ";
    SDL_Surface *surf = TTF_RenderUTF8_Blended(t->font, s, t->color);
    if (!surf) return;
    t->texture = SDL_CreateTextureFromSurface(r, surf);
    t->rect.w = surf->w;
    t->rect.h = surf->h;
    SDL_FreeSurface(surf);
}

void txt_init(SDL_Renderer *r, TxtLabel *t, int x, int y,
              Uint8 cr, Uint8 cg, Uint8 cb,
              const char *font_path, int sz, const char *txt)
{
    memset(t, 0, sizeof(*t));
    t->rect.x = x; t->rect.y = y;
    t->color  = (SDL_Color){cr, cg, cb, 255};
    t->font   = TTF_OpenFont(font_path, sz);
    if (!t->font) SDL_Log("TTF_OpenFont(%s): %s", font_path, TTF_GetError());
    strncpy(t->txt, txt ? txt : "", sizeof(t->txt)-1);
    txt_rerender(r, t);
}

void txt_set(SDL_Renderer *r, TxtLabel *t, const char *txt)
{
    if (!t) return;
    strncpy(t->txt, txt ? txt : "", sizeof(t->txt)-1);
    t->txt[sizeof(t->txt)-1] = '\0';
    txt_rerender(r, t);
}

void txt_render(SDL_Renderer *r, TxtLabel *t)
{
    if (t && t->texture) SDL_RenderCopy(r, t->texture, NULL, &t->rect);
}

void txt_free(TxtLabel *t)
{
    if (!t) return;
    if (t->texture) { SDL_DestroyTexture(t->texture); t->texture = NULL; }
    if (t->font)    { TTF_CloseFont(t->font); t->font = NULL; }
}

/* ==================== AUDIO ==================== */
void audio_switch(AudioCtx *a, int which)
{
    if (a->current == which) return;
    Mix_HaltMusic();
    a->current = which;
    Mix_Music *m = NULL;
    switch (which) {
        case 1: m = a->principal_music;  break;
        case 2: m = a->options_music;    break;
        case 3: m = a->sauvegarde_music; break;
        case 4: m = a->joueur_music;     break;
        case 5: m = a->score_music;      break;
        case 6: m = a->enigme_music;     break;
    }
    if (m) Mix_PlayMusic(m, -1);
}

void hover_once(AudioCtx *a, int *last, int cur)
{
    if (cur != *last) {
        if (cur != -1 && a->hover_snd)
            Mix_PlayChannel(-1, a->hover_snd, 0);
        *last = cur;
    }
}

/* ==================== DESSIN TEXTE ==================== */
void draw_text(SDL_Renderer *r, TTF_Font *f, const char *txt,
               int x, int y, SDL_Color c)
{
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    if (t) {
        SDL_Rect d = {x, y, s->w, s->h};
        SDL_RenderCopy(r, t, NULL, &d);
        SDL_DestroyTexture(t);
    }
    SDL_FreeSurface(s);
}

void draw_text_wrap(SDL_Renderer *r, TTF_Font *f, const char *txt,
                    int x, int y, int w, SDL_Color c)
{
    SDL_Surface *s = TTF_RenderUTF8_Blended_Wrapped(f, txt, c, w);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    if (t) {
        SDL_Rect d = {x, y, s->w, s->h};
        SDL_RenderCopy(r, t, NULL, &d);
        SDL_DestroyTexture(t);
    }
    SDL_FreeSurface(s);
}
