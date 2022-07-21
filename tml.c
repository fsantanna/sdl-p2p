#include <SDL.h>
#include <assert.h>
#include "tml.h"

#define MAX_EVT 10000
#define MAX_MEM 1000

typedef struct {
    int     tick;
    tml_evt evt;
} tml_tick_evt;

void tml_loop (int fps, int n, void* mem, void(*cb_sim)(tml_evt), void(*cb_eff)(int), int(*cb_rec)(SDL_Event*,tml_evt*), int(*cb_trv)(SDL_Event*,int,int,int*)) {
    char MEM[MAX_MEM][n];
    int mpf = 1000 / fps;
    assert(1000%fps == 0);

    struct {
        uint32_t nxt;
        int      mpf;
        int      tick;
    } S = { SDL_GetTicks()+mpf, mpf, 0 };

    struct {
        int tot;
        int nxt;
        tml_tick_evt buf[MAX_EVT];
    } Q = { 0, 0, {} };

    cb_sim((tml_evt) { TML_EVT_INIT });
    memcpy(MEM[0], mem, n);
    //printf("<<< memcpy %d\n", 0);

_RET_REC_: {

    //printf("REC %d\n", S.tick);
    while (1) {
        if (Q.nxt < Q.tot) {
            cb_sim(Q.buf[Q.nxt++].evt);
            cb_eff(0);
        } else {
            uint32_t now = SDL_GetTicks();
            if (now < S.nxt) {
                SDL_WaitEventTimeout(NULL, S.nxt-now);
                now = SDL_GetTicks();
            }
            if (now >= S.nxt) {
                S.tick++;
                S.nxt += S.mpf;
                cb_sim((tml_evt) { TML_EVT_TICK, {.tick=S.tick} });
                if (S.tick % 100 == 0) {
                    assert(MAX_MEM > S.tick/100);
                    memcpy(MEM[S.tick/100], mem, n);    // save w/o events
                    //printf("<<< memcpy %d\n", S.tick);
                }
                cb_eff(0);
            } else {
                SDL_Event sdl;
                tml_evt   evt;
                assert(SDL_PollEvent(&sdl));

                switch (cb_rec(&sdl, &evt)) {
                    case TML_RET_NONE:
                        break;
                    case TML_RET_QUIT:
                        return;
                    case TML_RET_REC:
                        assert(Q.tot < MAX_EVT);
                        Q.buf[Q.tot++] = (tml_tick_evt) { S.tick, evt };
                        break;
                    case TML_RET_TRV:
                        goto _RET_TRV_;
                }
            }
        }
    }
}

_RET_TRV_: {

    cb_eff(1);

    //printf("TRV %d\n", S.tick);
    uint32_t prv = SDL_GetTicks();
    uint32_t nxt = SDL_GetTicks();
    int tick = S.tick;
    int tot  = Q.tot;
    int new  = -1;

    while (1) {
        uint32_t now = SDL_GetTicks();
        if (now < nxt) {
            SDL_WaitEventTimeout(NULL, nxt-now);
            now = SDL_GetTicks();
        }
        if (now >= nxt) {
            nxt += S.mpf;
        }

        if (new != -1) {
            assert(0<=new && new<=S.tick);
            memcpy(mem, MEM[new/100], n);   // load w/o events
            int fst = new - new%100;
            //printf(">>> memcpy %d / fst %d\n", new/100, fst);

            // skip events before fst
            int e = 0;
            for (; e<Q.tot && Q.buf[e].tick<fst; e++);

            for (int i=fst; i<=new; i++) {
                if (i > fst) { // first tick already loaded
                    cb_sim((tml_evt) { TML_EVT_TICK, {.tick=i} });
                }
                while (e<Q.tot && Q.buf[e].tick==i) {
                    cb_sim(Q.buf[e].evt);
                    e++;
                }
            }
            tick = new;
            tot  = e;
            //SDL_Delay(S.mpf);
            cb_eff(1);
        }

        SDL_Event sdl;
        SDL_Event* ptr = SDL_PollEvent(&sdl) ? &sdl : NULL;

        switch (cb_trv(ptr, S.tick, tick, &new)) {
            case TML_RET_NONE:
                new = -1;
                break;
            case TML_RET_QUIT:
                return;
            case TML_RET_REC:
                S.nxt += (SDL_GetTicks() - prv);
                //printf("OUT %d\n", tick);
                S.tick = tick;
                Q.nxt = Q.tot = tot;
                //Q.nxt = MIN(Q.nxt, Q.tot);
                goto _RET_REC_;
                break;
            case TML_RET_TRV: {
                break;
            }
        }
    }
}
}
