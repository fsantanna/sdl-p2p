#if 0
#!/bin/sh
gcc -Wall -I ../tml/ `sdl2-config --cflags` ../tml/tml.c p2p.c move.c -o xmove `sdl2-config --libs` -lpthread -lSDL2_net
exit
#endif

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include "p2p.h"
#include "tml.h"

enum {
    TML_EVT_KEY = TML_EVT_NEXT
};

void cb_sim (tml_evt);
void cb_eff (int trv);
int  cb_rec (SDL_Event* sdl, tml_evt* evt);

#define FPS   50
#define WIN   400

#define VEL   3
#define DIM   10

struct {
    int x,  y;
    int dx, dy;
} G;

SDL_Renderer* REN = NULL;

int main (void) {
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);

    SDL_Window* win = SDL_CreateWindow (
        "TML: Time Machine Library",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIN, WIN,
        SDL_WINDOW_SHOWN
    );
    assert(win != NULL);

    REN = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    assert(REN != NULL);
    SDL_SetRenderDrawBlendMode(REN,SDL_BLENDMODE_BLEND);

    p2p_init(1,9999);
    tml_loop(50, sizeof(G), &G, cb_sim, cb_eff, cb_rec, NULL);
    p2p_quit();

    SDL_DestroyRenderer(REN);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void cb_sim (tml_evt evt) {
    switch (evt.id) {
        case TML_EVT_INIT:
            G.x  = 0;
            G.y  = 0;
            G.dx = 1;
            G.dy = 0;
            break;
        case TML_EVT_TICK:
            G.x += G.dx * VEL;
            G.y += G.dy * VEL;
            break;
        case TML_EVT_KEY:
            switch (evt.pay.i1) {
                case SDLK_UP:    { G.dx= 0; G.dy=-1; break; }
                case SDLK_DOWN:  { G.dx= 0; G.dy= 1; break; }
                case SDLK_LEFT:  { G.dx=-1; G.dy= 0; break; }
                case SDLK_RIGHT: { G.dx= 1; G.dy= 0; break; }
            }
            break;
    }
}

void cb_eff (int trv) {
    SDL_SetRenderDrawColor(REN, 0xFF,0xFF,0xFF,0xFF);
    SDL_RenderClear(REN);
    SDL_Rect r = { G.x, G.y, DIM, DIM };
    SDL_SetRenderDrawColor(REN, 0xFF,0x00,0x00,0xFF);
    SDL_RenderFillRect(REN, &r);
    SDL_RenderPresent(REN);
}

int cb_rec (SDL_Event* sdl, tml_evt* evt) {
    {
        uint8_t n;
        p2p_pay pay;
        if (p2p_step(&n, &pay)) {
            assert(n == 2);
            assert(pay.i2._1 == TML_EVT_KEY);
            *evt = (tml_evt) { pay.i2._1, {.i1=pay.i2._2} };
            return TML_RET_REC;
        }
    }
    
    switch (sdl->type) {
        case SDL_QUIT:
            return TML_RET_QUIT;
        case SDL_KEYDOWN: {
            int key = sdl->key.keysym.sym;
            p2p_pay pay = { .i2={TML_EVT_KEY,key} };
            p2p_bcast(2, &pay);
            break;
        }
    }
    return TML_RET_NONE;
}