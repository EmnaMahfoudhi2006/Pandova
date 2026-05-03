#include "../include/jeu_ennemi.h"
#include "../include/jeu_background.h"

#define GRAVITY_E  0.5f
#define DIST_RUN   400
#define DIST_WALK  200
#define DIST_ATK    90

/*
 * Robot  (idle/die/hit) : 1024x1024 → 4x4 → frame 256x256
 * Robot  (walk/run/atk) : 1280x1280 → 5x5 → frame 256x256
 * Human  (tout)         : 1280x1280 → 5x5 → frame 256x256
 *
 * col = frame % cols,  row = frame / cols
 * src.x = col * frame_w,  src.y = row * frame_h
 */
static const SprInfo ROBOT_INFO[E_COUNT]={
    {"assets/robot/idle.png",             5,5},
    {"assets/robot/routee-walk.png",      5,5},
    {"assets/robot/routee-run.png",       5,5},
    {"assets/robot/robote-attackfire.png",5,5},
    {"assets/robot/attack.png",           5,5},
    {"assets/robot/die.png",              5,5}
};
static const SprInfo HUMAN_INFO[E_COUNT]={
    {"assets/human/walk.png",      5,5},
    {"assets/human/walk.png",      5,5},
    {"assets/human/miles-run.png", 5,5},
    {"assets/human/attack.png",    5,5},
    {"assets/human/hitreact.png",  5,5},
    {"assets/human/die.png",       5,5}
};

SDL_Texture* ennemi_load_tex(const char *path, SDL_Renderer *r)
{
    SDL_Surface *s=IMG_Load(path);
    if(!s){SDL_Log("ennemi_load(%s):%s",path,IMG_GetError());return NULL;}
    SDL_Texture *t=SDL_CreateTextureFromSurface(r,s);
    SDL_FreeSurface(s); return t;
}

static void set_ea(Ennemi *e, EAction a)
{
    if(e->action!=a){e->action=a;e->frame=0;e->last_frame=SDL_GetTicks();}
}

static void next_frame_e(Ennemi *e, int delay)
{
    Uint32 now=SDL_GetTicks();
    int ai=(int)e->action;
    int total=e->cols[ai]*e->rows[ai];
    /* Si mort : rester bloqué sur la dernière frame */
    if(!e->alive){
        if(e->frame < total-1 && now-e->last_frame>=(Uint32)delay){
            e->frame++;
            e->last_frame=now;
        }
        return;
    }
    if(now-e->last_frame>=(Uint32)delay){
        e->frame=(e->frame+1)%total;
        e->last_frame=now;
    }
}

/*
 * Rendu sprite sheet ennemi
 *  cols et rows sont stockés par action dans e->cols[action], e->rows[action]
 *  frame_w = tex_w / cols,  frame_h = tex_h / rows
 *  col = frame % cols,  row = frame / cols
 */
static void draw_frame_e(SDL_Renderer *r, Ennemi *e, int sx, int sy)
{
    int ai=e->alive?(int)e->action:(int)E_DIE;
    SDL_Texture *tex=e->sprites[ai];
    if(!tex) return;
    int tw,th;
    SDL_QueryTexture(tex,NULL,NULL,&tw,&th);
    int nc=e->cols[ai], nr=e->rows[ai];
    int fw=tw/nc, fh=th/nr;
    int col=e->frame%nc, row=e->frame/nc;
    SDL_Rect src={col*fw,row*fh,fw,fh};
    SDL_Rect dst={sx,sy,DISPLAY_E,DISPLAY_E};
    SDL_RendererFlip flip=(e->dir>0)?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(r,tex,&src,&dst,0,NULL,flip);
}

void ennemi_init(Ennemi *e, EType type, int x, int y, SDL_Renderer *r)
{
    memset(e,0,sizeof(*e));
    e->type=type;
    /* Hitbox physique */
    e->pos=(SDL_Rect){x,y,PHYS_E,PHYS_E};
    e->dir=-1; e->alive=1;
    e->last_frame=SDL_GetTicks();

    const SprInfo *info=(type==TYPE_ROBOT)?ROBOT_INFO:HUMAN_INFO;
    if(type==TYPE_ROBOT){e->health=5;e->max_hp=5;e->speed=2.5f;e->attack_cd=1200;}
    else                {e->health=8;e->max_hp=8;e->speed=4.0f;e->attack_cd=800;}

    for(int i=0;i<E_COUNT;i++){
        e->sprites[i]=ennemi_load_tex(info[i].path,r);
        e->cols[i]=info[i].cols;
        e->rows[i]=info[i].rows;
    }
}

static SDL_Rect closest(Ennemi *e, SDL_Rect t1, SDL_Rect t2)
{
    int d1=abs(t1.x-e->pos.x), d2=abs(t2.x-e->pos.x);
    return (d1<=d2)?t1:t2;
}

void ennemi_update(Ennemi *e, SDL_Rect t1, SDL_Rect t2,
                   SDL_Surface *mask, int ground_y, int world_w, int world_h)
{
    (void)world_h;
    if(!e->alive){next_frame_e(e,100);return;}

    SDL_Rect tgt=closest(e,t1,t2);
    int dx=tgt.x-e->pos.x;
    float dist=(float)abs(dx);
    e->dir=(dx>0)?1:-1;

    if     (dist>DIST_RUN) set_ea(e,E_IDLE);
    else if(dist>DIST_WALK){set_ea(e,E_RUN);  e->pos.x+=(int)(e->speed*1.5f*e->dir);}
    else if(dist>DIST_ATK) {set_ea(e,E_WALK); e->pos.x+=(int)(e->speed*e->dir);}
    else                    set_ea(e,E_ATTACK);

    if(e->pos.x<0) e->pos.x=0;
    if(e->pos.x+e->pos.w>world_w) e->pos.x=world_w-e->pos.w;

    /* Gravité */
    e->vy+=GRAVITY_E;
    e->pos.y+=(int)e->vy;

    /* Sol masque noir */
    if(e->vy>0&&bg_col_feet(mask,e->pos)==1){
        while(bg_col_feet(mask,e->pos)==1&&e->pos.y>0) e->pos.y--;
        e->vy=0; e->on_ground=1;
    }
    /* Sol fixe */
    else if(e->pos.y+e->pos.h>=ground_y){
        e->pos.y=ground_y-e->pos.h;
        e->vy=0; e->on_ground=1;
    }

    if(e->health<=0){e->alive=0;set_ea(e,E_DIE);}

    int delay=(e->type==TYPE_ROBOT)?90:70;
    next_frame_e(e,delay);
}

void ennemi_render(SDL_Renderer *r, Ennemi *e,
                   int cam_x, int cam_y, int view_x, int view_w)
{
    /* Position hitbox à l'écran */
    int sx=(e->pos.x-cam_x)+view_x;
    int sy=(e->pos.y-cam_y);
    /* Offset pour centrer le grand sprite sur la hitbox */
    int rx=sx-DISP_OFF_EX;
    int ry=sy-DISP_OFF_EY;
    if(rx+DISPLAY_E<view_x||rx>view_x+view_w) return;

    SDL_Rect clip={view_x,0,view_w,720};
    SDL_RenderSetClipRect(r,&clip);
    draw_frame_e(r,e,rx,ry);

    if(e->alive){
        int bw=(int)((float)PHYS_E*e->health/e->max_hp);
        SDL_SetRenderDrawColor(r,140,0,0,255);
        SDL_Rect bg_b={sx,sy-12,PHYS_E,8}; SDL_RenderFillRect(r,&bg_b);
        SDL_SetRenderDrawColor(r,0,200,80,255);
        SDL_Rect hp_b={sx,sy-12,bw,8}; SDL_RenderFillRect(r,&hp_b);
    }
    SDL_RenderSetClipRect(r,NULL);
}

void ennemi_hit(Ennemi *e)
{
    if(!e->alive) return;
    e->health--;
    set_ea(e,E_HIT);
    if(e->health<=0){e->alive=0;set_ea(e,E_DIE);}
}

void ennemi_free(Ennemi *e)
{
    for(int i=0;i<E_COUNT;i++)
        if(e->sprites[i]){SDL_DestroyTexture(e->sprites[i]);e->sprites[i]=NULL;}
}
