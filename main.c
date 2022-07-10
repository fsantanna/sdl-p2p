#if 0
#!/bin/sh
gcc -g -Wall `sdl2-config --cflags` p2p.c main.c -o xmain `sdl2-config --libs` -lpthread -lSDL2_net
exit
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "p2p.h"

int main (int argc, char** argv) {
    assert(argc == 3);
    char me   = atoi(argv[1]);
    int  port = atoi(argv[2]);
    p2p_init(me,port);
    if (me == 2) {
        p2p_link("localhost", 5001, 1);
    }
    while (1) {
        p2p_step();
        if (rand()%500000 == 0) {
puts("-=-=-=-");
            static int i = 0;
            p2p_send(i++);
        }
    }
    p2p_quit();
    return 0;
}
