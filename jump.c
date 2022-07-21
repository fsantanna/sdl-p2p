#if 0
#!/bin/sh
gcc -g -Wall `sdl2-config --cflags` p2p.c jump.c -o xjump `sdl2-config --libs` -lpthread -lSDL2_net
exit
#endif

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include "p2p.h"
#include "tml.h"

enum {
    TML_EVT_JUMP = TML_EVT_NEXT
};

void cb_sim (p2p_evt);
void cb_eff (int trv);
int  cb_rec (SDL_Event* sdl, p2p_evt* evt);
int  cb_trv (SDL_Event* sdl, int max, int cur, int* ret);

#define FPS   50
#define WIN   400
#define FLOOR (2*WIN/3)
#define LAKEW 60
#define LAKEX (WIN/2-LAKEW/2)

#define DX    3
#define DYG   5
#define DYJ   -10
#define DIM   10

struct {
    int x, y;
    int dy;
    int dead;
} G;

SDL_Renderer* REN = NULL;

int main (void) {
    p2p_init(1,5001);
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

    tml_loop(50, sizeof(G), &G, cb_sim, cb_eff, cb_rec, cb_trv);

    SDL_DestroyRenderer(REN);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void cb_sim (p2p_evt evt) {
    switch (evt.id) {
        case TML_EVT_INIT:
            G.x    = 0;
            G.y    = FLOOR;
            G.dy   = DYG;
            G.dead = 0;
            break;
        case TML_EVT_TICK:
            if (!G.dead) {
                G.x  = (G.x + DX) % WIN;
                G.y  = MIN(FLOOR, G.y+G.dy);
                G.dy = MIN(DYG, G.dy+1);
                if ((G.y == FLOOR) && (LAKEX-DIM/2<=G.x && G.x<=LAKEX+LAKEW-DIM/2)) {
                    G.dead = evt.pay.i1;
                }
            }
            break;
        case TML_EVT_JUMP:
            if (!G.dead) {
                if (G.y == FLOOR) {
                    G.dy = DYJ;
                }
            }
            break;
    }
}

void cb_eff (int trv) {
    SDL_SetRenderDrawColor(REN, 0xFF,0xFF,0xFF,0xFF);
    SDL_RenderClear(REN);
    {
        // FLOOR
        SDL_Rect r = { 0, FLOOR+DIM, WIN, WIN-FLOOR-DIM };
        SDL_SetRenderDrawColor(REN, 0x4E,0x51,0x47,0xFF);
        SDL_RenderFillRect(REN, &r);
    }
    {
        // LAKE
        SDL_Rect r = { LAKEX, FLOOR+DIM, LAKEW, 2*DIM };
        SDL_SetRenderDrawColor(REN, 0x00,0x00,0xDD,0xFF);
        SDL_RenderFillRect(REN, &r);
    }
    {
        // PIXEL
        SDL_Rect r = { G.x, G.y, DIM, DIM };
        SDL_SetRenderDrawColor(REN, 0xFF,0x00,0x00,0xFF);
        SDL_RenderFillRect(REN, &r);
    }

    if (trv) {
        SDL_SetRenderDrawColor(REN, 0xFF,0xFF,0xFF,0x77);
        SDL_RenderFillRect(REN, NULL);
    }

    SDL_RenderPresent(REN);
}

int cb_rec (SDL_Event* sdl, p2p_evt* evt) {
    switch (sdl->type) {
        case SDL_QUIT:
            return TML_RET_QUIT;
        case SDL_KEYDOWN: {
            int key = sdl->key.keysym.sym;
            if (key==SDLK_UP) {
                *evt = (p2p_evt) { TML_EVT_JUMP };
                return TML_RET_REC;
            }
            if (key==SDLK_LEFT) {
                return TML_RET_TRV;
            }
            break;
        }
    }
    return TML_RET_NONE;
}

int cb_trv (SDL_Event* sdl, int max, int cur, int* ret) {
    if (G.dead) {
        cur = MIN(cur,G.dead);
    }

    if (sdl != NULL) {
        switch (sdl->type) {
            case SDL_QUIT:
                return TML_RET_QUIT;
            case SDL_KEYUP: {
                int key = sdl->key.keysym.sym;
                if (key == SDLK_LEFT) {
                    return TML_RET_REC;
                }
                break;
            }
        }
    }

    *ret = MAX(0, cur-1);
    return TML_RET_TRV;
}
