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

void p2p_bcast (uint32_t peer, uint32_t seq, uint8_t n, char* buf) {
    for (int i=0; i<G.clis_n; i++) {
        TCPsocket s = G.clis[i].s;
        tcp_send_u8 (s, peer);
        tcp_send_u32(s, seq);
        tcp_send_u8 (s, n);
        tcp_send_n  (s, n, buf);
    }
}

static void* f (void* arg) {
    p2p_t* cli = (p2p_t*) arg;
    //uint8_t peer = tcp_recv_u8(cli->s);

    while (1) {
        puts("cli");
        char buf[P2P_MAX];
        uint8_t  peer = tcp_recv_u8(cli->s);
        uint32_t seq  = tcp_recv_u32(cli->s);
        uint8_t  n    = tcp_recv_u8(cli->s);
        tcp_recv_n(cli->s, n, (char*) buf);
        printf("> packet > %d %d %d\n", peer, seq, n);

        if (peer!=G.srv.peer && seq>G.clis[peer].seq) {
            puts(">>> bcast >>>");
            p2p_bcast(peer, seq, n, buf);
        }
    }

    SDLNet_TCP_Close(cli->s);
    return NULL;
}

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

    pthread_t t;
    assert(pthread_create(&t, NULL, f, (void*) &G.clis[G.clis_n]) == 0);
    G.clis_n++;
}

void p2p_send (uint32_t v) {
    G.srv.seq++;
    v = htobe32(v);
    p2p_bcast(G.srv.peer, G.srv.seq, sizeof(uint32_t), (char*) &v);
}

void p2p_link (char* host, int port, uint8_t me) {
	IPaddress ip;
	assert(SDLNet_ResolveHost(&ip, host, port) == 0);
	TCPsocket s = SDLNet_TCP_Open(&ip);
    assert(s != NULL);
    G.clis[G.clis_n] = (p2p_t) { s, -1, 0 };
    //assert(SDLNet_TCP_Send(s,&me,1) == 1);
puts("---");

    pthread_t t;
    assert(pthread_create(&t, NULL, f, (void*) &G.clis[G.clis_n]) == 0);
    G.clis_n++;
}
