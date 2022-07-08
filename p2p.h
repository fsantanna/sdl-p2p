#define P2P_PORT 5000
#define P2P_MAX  32

#include <stdint.h>

typedef struct {
    uint32_t peer;
    uint32_t seq;
    struct {
        char n;
        char* buf;
    } pay;
} p2p_pak;

void p2p_init (void);
void p2p_quit (void);
void p2p_listen (void);
void p2p_step (void);
