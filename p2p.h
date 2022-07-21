#include <stdint.h>

typedef struct {
    uint8_t id;
    uint8_t n;
    union {
        int i1;
        struct {
            int _1,_2;
        } i2;
        struct {
            int _1,_2,_3;
        } i3;
        struct {
            int _1,_2,_3,_4;
        } i4;
    } pay;
} p2p_evt;

typedef struct {
    char     status;    // 0=receiving, -1=repeated, 1=ok
    uint8_t  src;
    uint32_t seq;
    uint32_t tick;
    p2p_evt  evt;
} p2p_pak;

void p2p_init  (uint8_t me, int port);
void p2p_quit  (void);
int  p2p_step  (p2p_evt* evt);
void p2p_bcast (uint32_t tick, p2p_evt* evt);
void p2p_link  (char* host, int port, uint8_t me);
void p2p_dump  (void);
