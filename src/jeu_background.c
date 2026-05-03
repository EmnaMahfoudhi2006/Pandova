#include "../include/jeu_background.h"

SDL_Color bg_get_pixel(SDL_Surface *mask, int x, int y)
{
    SDL_Color color = {128,128,128,255};
    if (!mask) return color;
    if (x<0||x>=mask->w||y<0||y>=mask->h) return color;
    Uint8 *p = (Uint8*)mask->pixels + y*mask->pitch + x*mask->format->BytesPerPixel;
    Uint32 pv = 0;
    memcpy(&pv, p, mask->format->BytesPerPixel);
    SDL_GetRGB(pv, mask->format, &color.r, &color.g, &color.b);
    return color;
}

static int classify(SDL_Color c)
{
    if (c.r==0   && c.g==0   && c.b==0  ) return 1;  /* noir  = sol/mur    */
    if (c.r==255 && c.g==255 && c.b==255) return 2;  /* blanc = danger     */
    return 0;
}

/* Teste les 8 points (tous) */
int bg_collision(SDL_Surface *mask, SDL_Rect pos)
{
    int X=pos.x, Y=pos.y, W=pos.w, H=pos.h;
    SDL_Point pts[8]={
        {X,Y},{X+W/2,Y},{X+W,Y},
        {X,Y+H/2},{X+W,Y+H/2},
        {X,Y+H},{X+W/2,Y+H},{X+W,Y+H}
    };
    for(int i=0;i<8;i++){
        int r=classify(bg_get_pixel(mask,pts[i].x,pts[i].y));
        if(r) return r;
    }
    return 0;
}

/* Teste seulement les 3 points bas (pieds) */
int bg_col_feet(SDL_Surface *mask, SDL_Rect pos)
{
    int X=pos.x, Y=pos.y+pos.h, W=pos.w;
    int r;
    r=classify(bg_get_pixel(mask,X,      Y)); if(r) return r;
    r=classify(bg_get_pixel(mask,X+W/2,  Y)); if(r) return r;
    r=classify(bg_get_pixel(mask,X+W,    Y)); if(r) return r;
    return 0;
}

/* Teste seulement les points latéraux */
int bg_col_sides(SDL_Surface *mask, SDL_Rect pos)
{
    int X=pos.x, Y=pos.y, W=pos.w, H=pos.h;
    int r;
    r=classify(bg_get_pixel(mask,X,   Y+H/2)); if(r) return r;
    r=classify(bg_get_pixel(mask,X+W, Y+H/2)); if(r) return r;
    return 0;
}

/* Teste les 3 points haut */
int bg_col_top(SDL_Surface *mask, SDL_Rect pos)
{
    int X=pos.x, Y=pos.y, W=pos.w;
    int r;
    r=classify(bg_get_pixel(mask,X,     Y)); if(r) return r;
    r=classify(bg_get_pixel(mask,X+W/2, Y)); if(r) return r;
    r=classify(bg_get_pixel(mask,X+W,   Y)); if(r) return r;
    return 0;
}

void bg_init(Background *b, SDL_Renderer *r)
{
    memset(b,0,sizeof(*b));
    b->levels[0].tex      = IMG_LoadTexture(r,"assets/backgrounds/bglvl1.png");
    b->levels[0].mask     = SDL_LoadBMP("assets/backgrounds/bglvl1mask.bmp");
    b->levels[0].width    = BG_LVL1_W;
    b->levels[0].height   = BG_LVL1_H;
    b->levels[0].ground_y = GROUND_LVL1;
    if(!b->levels[0].tex)  SDL_Log("bglvl1.png: %s",IMG_GetError());
    if(!b->levels[0].mask) SDL_Log("bglvl1mask.bmp: %s",SDL_GetError());

    b->levels[1].tex      = IMG_LoadTexture(r,"assets/backgrounds/bglvl2.png");
    b->levels[1].mask     = SDL_LoadBMP("assets/backgrounds/bglvl2mask.bmp");
    b->levels[1].width    = BG_LVL2_W;
    b->levels[1].height   = BG_LVL2_H;
    b->levels[1].ground_y = GROUND_LVL2;
    if(!b->levels[1].tex)  SDL_Log("bglvl2.png: %s",IMG_GetError());
    if(!b->levels[1].mask) SDL_Log("bglvl2mask.bmp: %s",SDL_GetError());

    b->cur=0;
}

void bg_switch_level(Background *b, int lvl)
{ if(lvl>=0&&lvl<2) b->cur=lvl; }

void bg_free(Background *b)
{
    for(int i=0;i<2;i++){
        if(b->levels[i].tex)  { SDL_DestroyTexture(b->levels[i].tex); b->levels[i].tex=NULL; }
        if(b->levels[i].mask) { SDL_FreeSurface(b->levels[i].mask);   b->levels[i].mask=NULL; }
    }
}

void bg_render(SDL_Renderer *r, Background *b,
               int cam_x, int cam_y,
               int dst_x, int dst_y, int dst_w, int dst_h)
{
    Level *lv=&b->levels[b->cur];
    if(!lv->tex) return;
    if(cam_x<0) cam_x=0;
    if(cam_y<0) cam_y=0;
    if(cam_x+dst_w>lv->width)  cam_x=lv->width -dst_w;
    if(cam_y+dst_h>lv->height) cam_y=lv->height-dst_h;
    SDL_Rect src={cam_x,cam_y,dst_w,dst_h};
    SDL_Rect dst={dst_x,dst_y,dst_w,dst_h};
    SDL_RenderCopy(r,lv->tex,&src,&dst);
}
