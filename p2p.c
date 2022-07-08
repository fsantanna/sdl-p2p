#include <endian.h>
#include <SDL2/SDL_net.h>
#include <assert.h>

#include "p2p.h"

typedef struct {
    TCPsocket s;
    int seqno;
    int n;
    char buf[P2P_MAX];
} p2p_t;

static TCPsocket S = NULL;

#define CS_N 16
static int CS_n = 0;
static p2p_t CS[CS_N];

void p2p_init (void) {
	assert(SDLNet_Init() == 0);
}

void p2p_quit (void) {
    for (int i=0; i<CS_n; i++) {
        SDLNet_TCP_Close(CS[i].s);
    }
	SDLNet_TCP_Close(S);
	SDLNet_Quit();
}

void p2p_listen (void) {
	IPaddress ip;
	assert(SDLNet_ResolveHost(&ip, NULL, P2P_PORT) == 0);
	S = SDLNet_TCP_Open(&ip);
	assert(S != NULL);
}

static int p2p_accept (void) {
    TCPsocket s = SDLNet_TCP_Accept(S);
    if (s == NULL) {
        return 0;
    }
    IPaddress* ip = SDLNet_TCP_GetPeerAddress(s);
    assert(ip != NULL);
    assert(CS_n < CS_N);
    CS[CS_n++] = (p2p_t) { s, 0, 0, {} };
    return 1;
}

void p2p_step (void) {
    while (p2p_accept());

    for (int i=0; i<CS_n; i++) {
        p2p_t* v = &CS[i];

        // fist byte with payload size
        int ok = (v->n != 0);
        if (!ok) {
            ok = SDLNet_TCP_Recv(v->s, &v->buf[0], 1);
            if (ok) {
                v->n++;
                assert(v->buf[0] < P2P_MAX);
            }
        }

        // remaining bytes until complete
        if (ok) {
            int n = SDLNet_TCP_Recv(v->s, &v->buf[v->n], v->buf[0]-(v->n));
            if (n > 0) {
                v->n += n;
                if (v->n == v->buf[0]) {
                    v->n = 0;
                    p2p_pak pak;
                    int n = 1;
                    pak.peer = be32toh(*(uint32_t*)&v->buf[n]);
                    n += sizeof(uint32_t);
                    pak.seq = be32toh(*(uint32_t*)&v->buf[n]);
                    n += sizeof(uint32_t);
                    pak.pay.n = v->buf[n];
                    n += 1;
                    pak.pay.buf = &v->buf[n];
                    printf(">>> %d %d %d", pak.peer, pak.seq, pak.pay.n);
                }
            }
        }
    }
}

void tcp_send_s32 (int v) {
    uint32_t _v = htobe32(v);
    assert(SDLNet_TCP_Send(S, &_v, sizeof(_v)) == sizeof(_v));
}

