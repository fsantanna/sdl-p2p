#include <stdint.h>

typedef union {
    uint32_t i1;
    struct {
        uint32_t _1,_2;
    } i2;
    struct {
        uint32_t _1,_2,_3;
    } i3;
    struct {
        uint32_t _1,_2,_3,_4;
    } i4;
} p2p_pay;

typedef struct {
    uint8_t  src;
    uint32_t seq;
    uint8_t  n;
    p2p_pay  pay;
} p2p_pak;

void p2p_init  (uint8_t me, int port);
void p2p_quit  (void);
int  p2p_step  (uint8_t* n, p2p_pay* pay);
void p2p_bcast (uint8_t n, p2p_pay* pay);
void p2p_link  (char* host, int port, uint8_t me);
void p2p_dump  (void);
