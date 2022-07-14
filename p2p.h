#define P2P_MAX 32

#include <stdint.h>

typedef struct {
    uint8_t  src;
    uint32_t seq;
    uint8_t  n;
    char     buf[P2P_MAX];
} p2p_pak;

void p2p_init (uint8_t me, int port, void(*cb)(uint8_t,char*));
void p2p_quit (void);
void p2p_step (void);
void p2p_send (uint32_t v);
void p2p_link (char* host, int port, uint8_t me);
void p2p_dump (void);
