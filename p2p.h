#define P2P_MAX 32

#include <stdint.h>

typedef struct {
    uint8_t  src;
    uint32_t seq;
    uint8_t  n;
    char     buf[P2P_MAX];
} p2p_pak;

void p2p_init  (uint8_t me, int port);
void p2p_quit  (void);
int  p2p_step  (uint8_t* n, char** buf);
void p2p_bcast (uint32_t v);
void p2p_link  (char* host, int port, uint8_t me);
void p2p_dump  (void);
