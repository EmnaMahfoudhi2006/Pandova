#include "../include/jeu_personage.h"
#include "../include/jeu_background.h"

/* Chemins sprites par personnage [id][action] */
static const char *SPR_PATH[2][A_COUNT]={
    {"assets/personage1/idle.png","assets/personage1/walk.png",
     "assets/personage1/run.png", "assets/personage1/jump.png",
     "assets/personage1/attack.png","assets/personage1/die.png"},
    {"assets/personage2/idle.png","assets/personage2/walk.png",
     "assets/personage2/run.png", "assets/personage2/jump.png",
     "assets/personage2/attack.png","assets/personage2/die.png"}
};

SDL_Texture* perso_load_tex(const char *path, SDL_Renderer *r)
{
    SDL_Surface *s=IMG_Load(path);
    if(!s){SDL_Log("perso_load(%s):%s",path,IMG_GetError());return NULL;}
    SDL_Texture *t=SDL_CreateTextureFromSurface(r,s);
    SDL_FreeSurface(s); return t;
}

static void set_action(Personage *p, PAction a)
{
    if(p->action!=a){p->action=a;p->frame=0;p->last_frame=SDL_GetTicks();}
}

static void next_frame(Personage *p)
{
    Uint32 now=SDL_GetTicks();
    if(now-p->last_frame>=FRAME_DELAY){
        p->frame=(p->frame+1)%SPR_FRAMES;
        if(p->is_attacking&&p->frame==0){p->is_attacking=0;p->hit_dealt=0;set_action(p,A_IDLE);}
        p->last_frame=now;
    }
}

/*
 * Rendu d'une frame sprite sheet
 *  grille 5 colonnes × 5 lignes = 25 frames
 *  frame_w = tex_w / 5 = 256
 *  col = frame % 5,  row = frame / 5
 *  src.x = col*256,  src.y = row*256
 */
static void draw_frame(SDL_Renderer *r, Personage *p, int sx, int sy)
{
    SDL_Texture *tex=p->sprites[p->action];
    if(!tex) return;
    int tw,th;
    SDL_QueryTexture(tex,NULL,NULL,&tw,&th);
    int fw=tw/SPR_COLS, fh=th/SPR_ROWS;
    int col=p->frame%SPR_COLS, row=p->frame/SPR_COLS;
    SDL_Rect src={col*fw,row*fh,fw,fh};
    SDL_Rect dst={sx,sy,DISPLAY_W,DISPLAY_H};
    SDL_RendererFlip flip=p->facing_right?SDL_FLIP_NONE:SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(r,tex,&src,&dst,0,NULL,flip);
}

void perso_init(Personage *p, int id, int x, int ground_y, SDL_Renderer *r)
{
    memset(p,0,sizeof(*p));
    p->id=id; p->hp=3; p->alive=1; p->facing_right=1;
    /* Placer joueur sur le sol : bas du sprite = ground_y */
    /* Hitbox = PHYS_W x PHYS_H, identique v4 */
    p->pos=(SDL_Rect){x, ground_y-PHYS_H, PHYS_W, PHYS_H};
    p->on_ground=1;
    p->action=A_IDLE; p->last_frame=SDL_GetTicks();
    for(int a=0;a<A_COUNT;a++)
        p->sprites[a]=perso_load_tex(SPR_PATH[id][a],r);
    p->heart_tex=perso_load_tex("assets/hearts/heart.png",r);
}

void perso_update(Personage *p,
                  int right, int left, int run,
                  int jump, int super_jump, int attack,
                  SDL_Surface *mask, int ground_y,
                  int world_w, int world_h __attribute__((unused)))
{
    if(!p->alive){next_frame(p);return;}

    /* Invincibilité temporaire après dégât */
    if(p->invincible && SDL_GetTicks()-p->hurt_time>1500)
        p->invincible=0;

    /* ── Horizontal ── */
    int speed=run?RUN_SPEED:WALK_SPEED;
    if(right){p->pos.x+=speed; p->facing_right=1;}
    if(left) {p->pos.x-=speed; p->facing_right=0;}
    if(p->pos.x<0) p->pos.x=0;
    if(p->pos.x+p->pos.w>world_w) p->pos.x=world_w-p->pos.w;

    /* Collision latérale masque (murs noirs) */
    if(bg_col_sides(mask,p->pos)==1){
        if(right) p->pos.x-=speed;
        if(left)  p->pos.x+=speed;
    }

    /* ── Gravité ── */
    p->vy+=GRAVITY;
    p->pos.y+=(int)p->vy;

    /* ── Collision verticale ── */
    /* 1) Tombe sur sol noir du masque */
    if(p->vy>0 && bg_col_feet(mask,p->pos)==1){
        while(bg_col_feet(mask,p->pos)==1&&p->pos.y>0) p->pos.y--;
        p->vy=0; p->on_ground=1; p->is_jumping=0;
    }
    /* 2) Sol fixe du niveau (herbe visible) */
    else if(p->pos.y+p->pos.h>=ground_y){
        p->pos.y=ground_y-p->pos.h;
        p->vy=0; p->on_ground=1; p->is_jumping=0;
    }
    /* 3) Plafond noir */
    else if(p->vy<0 && bg_col_top(mask,p->pos)==1){
        while(bg_col_top(mask,p->pos)==1) p->pos.y++;
        p->vy=0;
    }
    /* 4) En l'air : vérifier si toujours sur le sol */
    else{
        /* Si pas de noir sous les pieds ET au-dessus du sol fixe → en l'air */
        SDL_Rect feet={p->pos.x+10,p->pos.y+p->pos.h,p->pos.w-20,2};
        if(bg_col_feet(mask,feet)==0 && p->pos.y+p->pos.h<ground_y)
            p->on_ground=0;
    }

    /* 5) Zone blanche = danger (uniquement LVL2) */
    if(bg_col_feet(mask,p->pos)==2 && !p->invincible){
        p->hp--;
        p->invincible=1; p->hurt_time=SDL_GetTicks();
        p->vy=JUMP_VEL/2; /* petit rebond */
        if(p->hp<=0){p->alive=0;set_action(p,A_DIE);}
    }

    /* Clamp vertical bas */
    if(p->pos.y<0){p->pos.y=0;p->vy=0;}

    /* ── Saut ── */
    if((jump||super_jump)&&p->on_ground){
        p->vy=super_jump?SUPER_JUMP:JUMP_VEL;
        p->on_ground=0; p->is_jumping=1; p->frame=0;
    }

    /* ── Attaque ── */
    if(attack&&!p->is_attacking){
        p->is_attacking=1;
        p->hit_dealt=0;    /* reset : prêt à infliger 1 seul dégât */
        set_action(p,A_ATTACK);
    }

    /* ── Action animation ── */
    if(!p->is_attacking&&p->alive){
        if(p->is_jumping||!p->on_ground) set_action(p,A_JUMP);
        else if(right||left)             set_action(p,run?A_RUN:A_WALK);
        else                             set_action(p,A_IDLE);
    }
    if(!p->alive) set_action(p,A_DIE);

    next_frame(p);
}

void perso_render(SDL_Renderer *r, Personage *p,
                  int cam_x, int cam_y, int view_x, int view_w)
{
    /* Clignotement si invincible */
    if(p->invincible&&(SDL_GetTicks()/100)%2) return;

    /* Position hitbox à l'écran */
    int sx=(p->pos.x-cam_x)+view_x;
    int sy=(p->pos.y-cam_y);
    /* Offset : centrer le grand sprite (480x480) sur la hitbox (80x160) */
    int rx=sx-DISP_OFF_X;
    int ry=sy-DISP_OFF_Y;
    /* Culling */
    if(rx+DISPLAY_W<view_x||rx>view_x+view_w) return;

    SDL_Rect clip={view_x,0,view_w,720};
    SDL_RenderSetClipRect(r,&clip);
    draw_frame(r,p,rx,ry);
    SDL_RenderSetClipRect(r,NULL);
}

void perso_render_hud(SDL_Renderer *r, Personage *p, int hud_x, TTF_Font *font)
{
    /* Cœurs */
    for(int i=0;i<p->hp&&i<5;i++){
        SDL_Rect hr={hud_x+i*40,8,34,34};
        if(p->heart_tex) SDL_RenderCopy(r,p->heart_tex,NULL,&hr);
    }
    /* Score */
    if(font){
        char buf[32]; snprintf(buf,sizeof(buf),"Score: %d",p->score);
        SDL_Color c={255,255,255,255};
        SDL_Surface *s=TTF_RenderText_Blended(font,buf,c);
        if(s){ SDL_Texture *t=SDL_CreateTextureFromSurface(r,s);
               SDL_Rect d={hud_x,48,s->w,s->h};
               SDL_RenderCopy(r,t,NULL,&d);
               SDL_DestroyTexture(t);SDL_FreeSurface(s); }
    }
}

void perso_free(Personage *p)
{
    for(int a=0;a<A_COUNT;a++)
        if(p->sprites[a]){SDL_DestroyTexture(p->sprites[a]);p->sprites[a]=NULL;}
    if(p->heart_tex){SDL_DestroyTexture(p->heart_tex);p->heart_tex=NULL;}
}
