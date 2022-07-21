#include <assert.h>
#include <pthread.h>
#include <endian.h>
#include <SDL2/SDL_net.h>

#include "p2p.h"
#include "tcp.c"

#define NET_N  32
#define PAKS_N 65536

typedef struct {
    TCPsocket s;
    uint32_t seq;
} p2p_net;

static uint8_t ME = -1;

static p2p_net NET[NET_N];

static int PAKS_i = 0;
static int PAKS_n = 0;
static p2p_pak PAKS[PAKS_N];

static pthread_mutex_t L;
#define LOCK()   pthread_mutex_lock(&L)
#define UNLOCK() pthread_mutex_unlock(&L);

void p2p_init (uint8_t me, int port) {
    assert(me < NET_N);
    ME = me;
    for (int i=0; i<NET_N; i++) {
        NET[i] = (p2p_net) { NULL, 0 };
    }
    assert(pthread_mutex_init(&L,NULL) == 0);
    assert(SDLNet_Init() == 0);
    IPaddress ip;
    assert(SDLNet_ResolveHost(&ip, NULL, port) == 0);
    TCPsocket s = SDLNet_TCP_Open(&ip);
    assert(s != NULL);
    NET[ME] = (p2p_net) { s, 0 };
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

static void p2p_bcast2 (p2p_pak* pak) {
    for (int i=0; i<NET_N; i++) {
        if (i == ME) continue;
        TCPsocket s = NET[i].s;
        if (s == NULL) continue;
        LOCK();
        tcp_send_u8 (s, pak->src);
        tcp_send_u32(s, pak->seq);
        tcp_send_u8 (s, pak->n);
        tcp_send_n  (s, pak->n*sizeof(uint32_t), (char*)&pak->pay);
        UNLOCK();
    }
}

static void* f (void* arg) {
    TCPsocket s = (TCPsocket) arg;
    LOCK();
    tcp_send_u8(s, ME);
    UNLOCK();
    uint8_t oth = tcp_recv_u8(s);

    LOCK();
    assert(oth < NET_N);
    assert(NET[oth].s == NULL);
    NET[oth] = (p2p_net) { s, 0 };
    UNLOCK();

    for (int i=0; i<PAKS_n; i++) {
        if (PAKS[i].seq == 0) {
            break;
        }
        p2p_bcast2(&PAKS[i]);
    }

    while (1) {
        LOCK();
        assert(PAKS_n < PAKS_N);
        p2p_pak* pak = &PAKS[PAKS_n++];
        pak->seq = 0;   // not ready
        UNLOCK();

        uint8_t  src = tcp_recv_u8(s);
        uint32_t seq = tcp_recv_u32(s);
        uint8_t  n   = tcp_recv_u8(s);
        *pak = (p2p_pak) { src, 0, n, {} };
        tcp_recv_n(s, n*sizeof(uint32_t), (char*)&pak->pay);

        LOCK();
//printf("+++++ %d %d %d\n", src, seq, n);
        pak->seq = seq; // now ready
        UNLOCK();

        LOCK();
        int cur = NET[src].seq;
        if (seq > cur) {
            assert(seq == cur+1);
            NET[src].seq++;
        }
        UNLOCK();

        if (seq > cur) {
            p2p_bcast2(pak);
        }
    }

    SDLNet_TCP_Close(s);
    return NULL;
}

int p2p_step  (uint8_t* n, p2p_pay* pay) {
    while (1) {
        TCPsocket s = SDLNet_TCP_Accept(NET[ME].s);
        if (s == NULL) {
            break;
        } else {
            IPaddress* ip = SDLNet_TCP_GetPeerAddress(s);
            assert(ip != NULL);
            pthread_t t;
            assert(pthread_create(&t, NULL,f,(void*)s) == 0);
        }
    }
    if (PAKS_i<PAKS_n && PAKS[PAKS_i].seq>0) {
        *n = PAKS[PAKS_i].n;
        pay->i4._1 = be32toh(PAKS[PAKS_i].pay.i4._1);
        pay->i4._2 = be32toh(PAKS[PAKS_i].pay.i4._2);
        pay->i4._3 = be32toh(PAKS[PAKS_i].pay.i4._3);
        pay->i4._4 = be32toh(PAKS[PAKS_i].pay.i4._4);
        PAKS_i++;
        return 1;
    }
    return 0;
}

void p2p_bcast (uint8_t n, p2p_pay* pay) {
    LOCK();
    uint32_t seq = ++NET[ME].seq;
    assert(PAKS_n < PAKS_N);
    p2p_pak* pak = &PAKS[PAKS_n++];
    UNLOCK();
    *pak = (p2p_pak) { ME, seq, n, {} };
    pak->pay.i4._1 = htobe32(pay->i4._1);
    pak->pay.i4._2 = htobe32(pay->i4._2);
    pak->pay.i4._3 = htobe32(pay->i4._3);
    pak->pay.i4._4 = htobe32(pay->i4._4);
    p2p_bcast2(pak);
}

void p2p_link (char* host, int port, uint8_t oth) {
	IPaddress ip;
	assert(SDLNet_ResolveHost(&ip, host, port) == 0);
	TCPsocket s = SDLNet_TCP_Open(&ip);
    assert(s != NULL);
    pthread_t t;
    assert(pthread_create(&t, NULL,f,(void*)s) == 0);
}

void p2p_dump (void) {
    for (int i=0; i<5; i++) {
        if (NET[i].s == NULL) {
            printf("- ");
        } else {
            printf("%d ", NET[i].seq);
        }
    }
    printf("\n");
}
