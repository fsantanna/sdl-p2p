#include <assert.h>
#include <pthread.h>
#include <endian.h>
#include <SDL2/SDL_net.h>

#include "p2p.h"
#include "tcp.c"

#define NET_N 32

typedef struct {
    TCPsocket s;
    uint32_t seq;
} p2p_t;

p2p_t NET[NET_N];
uint8_t ME = -1;

static pthread_mutex_t L;
#define LOCK()   pthread_mutex_lock(&L)
#define UNLOCK() pthread_mutex_unlock(&L);

void p2p_init (uint8_t me, int port) {
    assert(me < NET_N);
    ME = me;
    for (int i=0; i<NET_N; i++) {
        NET[i] = (p2p_t) { NULL, 0 };
    }
    assert(pthread_mutex_init(&L,NULL) == 0);
    assert(SDLNet_Init() == 0);
    IPaddress ip;
    assert(SDLNet_ResolveHost(&ip, NULL, port) == 0);
    TCPsocket s = SDLNet_TCP_Open(&ip);
    assert(s != NULL);
    NET[ME] = (p2p_t) { s, 0 };
}

void p2p_quit (void) {
    LOCK();
    for (int i=0; i<NET_N; i++) {
        SDLNet_TCP_Close(NET[i].s);
    }
    UNLOCK();
    pthread_mutex_destroy(&L);
	SDLNet_Quit();
}

void p2p_bcast (uint8_t src, uint32_t seq, uint8_t n, char* buf) {
    for (int i=0; i<NET_N; i++) {
        if (i == ME) continue;
        TCPsocket s = NET[i].s;
        if (s == NULL) continue;
        tcp_send_u8 (s, src);
        tcp_send_u32(s, seq);
        tcp_send_u8 (s, n);
        tcp_send_n  (s, n, buf);
    }
}

static void* f (void* arg) {
    TCPsocket s = (TCPsocket) arg;
    uint8_t oth = tcp_recv_u8(s);

    LOCK();
    assert(oth < NET_N);
    assert(NET[oth].s == NULL);
    NET[oth] = (p2p_t) { s, 0 };
    UNLOCK();

    while (1) {
        char buf[P2P_MAX];
        uint8_t  src = tcp_recv_u8(s);
        uint32_t seq = tcp_recv_u32(s);
        uint8_t  n   = tcp_recv_u8(s);
        tcp_recv_n(s, n, (char*) buf);
        printf("> packet > %d %d %d\n", src, seq, n);

        LOCK();
        int cur = NET[src].seq;
        if (seq > cur) {
            assert(seq == cur+1);
            NET[src].seq++;
        }
        UNLOCK();

        if (seq > cur) {
            p2p_bcast(src, seq, n, buf);
        }
    }

    SDLNet_TCP_Close(s);
    return NULL;
}

void p2p_step (void) {
    TCPsocket s = SDLNet_TCP_Accept(NET[ME].s);
    if (s != NULL) {
        IPaddress* ip = SDLNet_TCP_GetPeerAddress(s);
        assert(ip != NULL);
        tcp_send_u8(s, ME);
        pthread_t t;
        assert(pthread_create(&t, NULL,f,(void*)s) == 0);
    }
}

void p2p_send (uint32_t v) {
    LOCK();
    NET[ME].seq++;
    uint32_t seq = NET[ME].seq;
    UNLOCK();
    v = htobe32(v);
    p2p_bcast(ME, seq, sizeof(uint32_t), (char*) &v);
}

void p2p_link (char* host, int port, uint8_t oth) {
	IPaddress ip;
	assert(SDLNet_ResolveHost(&ip, host, port) == 0);
	TCPsocket s = SDLNet_TCP_Open(&ip);
    assert(s != NULL);
    tcp_send_u8(s, ME);
    pthread_t t;
    assert(pthread_create(&t, NULL,f,(void*)s) == 0);
}
