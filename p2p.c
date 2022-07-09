#include <assert.h>
#include <pthread.h>
#include <endian.h>
#include <SDL2/SDL_net.h>

#include "p2p.h"
#include "tcp.c"

#define CS_N 16

typedef struct {
    TCPsocket s;
    uint8_t peer;
    uint32_t seq;
} p2p_t;

struct {
    p2p_t srv;
    int clis_n;
    p2p_t clis[CS_N];
} G = { {NULL,-1,0}, 0, {} };

void p2p_init (uint8_t peer, int port) {
    assert(SDLNet_Init() == 0);
    IPaddress ip;
    assert(SDLNet_ResolveHost(&ip, NULL, port) == 0);
    G.srv.s = SDLNet_TCP_Open(&ip);
    assert(G.srv.s != NULL);
    G.srv.peer = peer;
}

void p2p_quit (void) {
	SDLNet_TCP_Close(G.srv.s);
	SDLNet_Quit();
}

void p2p_step (void) {
    TCPsocket s = SDLNet_TCP_Accept(G.srv.s);
    if (s == NULL) return;

    puts("> new conn >");

    IPaddress* ip = SDLNet_TCP_GetPeerAddress(s);
    assert(ip != NULL);
    assert(G.clis_n < CS_N);
    G.clis[G.clis_n] = (p2p_t) { s, -1, 0 };

    void* f (void* arg) {
        p2p_t* cli = (p2p_t*) arg;
        uint8_t peer = tcp_recv_u8(s);

        puts("cli");
        while (1) {
            p2p_pak pak;
            pak.peer = tcp_recv_u8(s);
            pak.seq  = tcp_recv_u32(s);
            pak.pay.n = tcp_recv_u8(s);
            tcp_recv_n(s, pak.pay.n, &pak.pay.buf);
            printf("> packet > %d %d %d", pak.peer, pak.seq, pak.pay.n);
        }

        SDLNet_TCP_Close(s);
        return NULL;
    }

    pthread_t t;
    assert(pthread_create(&t, NULL, f, (void*) &G.clis[G.clis_n]) == 0);
    G.clis_n++;
}

void p2p_send (uint32_t v) {
    G.srv.seq++;
    char n1 = 1 + 1 + sizeof(uint32_t) + 1 + sizeof(uint32_t);
    char n2 = sizeof(uint32_t);
    uint32_t seq = htobe32(G.srv.seq);
    v = htobe32(v);
    for (int i=0; i<G.clis_n; i++) {
        TCPsocket s = G.clis[i].s;
        assert(SDLNet_TCP_Send(s, &n1,      1)                == 1);
        assert(SDLNet_TCP_Send(s, &G.srv.peer, 1)                == 1);
        assert(SDLNet_TCP_Send(s, &seq,     sizeof(uint32_t)) == sizeof(uint32_t));
        assert(SDLNet_TCP_Send(s, &n2,      1)                == 1);
        assert(SDLNet_TCP_Send(s, &v,       sizeof(uint32_t)) == sizeof(uint32_t));
    }
}

void p2p_link (char* host, int port, uint8_t me) {
	IPaddress ip;
	assert(SDLNet_ResolveHost(&ip, host, port) == 0);
	TCPsocket s = SDLNet_TCP_Open(&ip);
    assert(s != NULL);
    G.clis[G.clis_n++] = (p2p_t) { s, -1, 0 };
    assert(SDLNet_TCP_Send(s,&me,1) == 1);
puts("---");
}
