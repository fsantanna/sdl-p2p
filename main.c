#if 0
#!/bin/sh
gcc -Wall `sdl2-config --cflags` p2p.c main.c -o xmain `sdl2-config --libs` -l SDL2_net
exit
#endif

#include "p2p.h"

int main (void) {
    p2p_init();
    p2p_listen();
    while (1) {
        p2p_step();
    }
    p2p_quit();
    return 0;
}
