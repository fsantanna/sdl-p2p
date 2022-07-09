#define P2P_MAX 32

#include <stdint.h>

typedef struct {
    uint8_t peer;
    uint32_t seq;
    struct {
        uint8_t n;
        char buf[P2P_MAX];
    } pay;
} p2p_pak;

void p2p_init (uint8_t peer, int port);
void p2p_quit (void);
void p2p_step (void);
void p2p_send (uint32_t v);
void p2p_link (char* host, int port, uint8_t me);
