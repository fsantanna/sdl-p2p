#if 0
#!/bin/sh
gcc -g -Wall `sdl2-config --cflags` p2p.c main.c -o xmain `sdl2-config --libs` -lpthread -lSDL2_net
exit
#endif

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "p2p.h"

/*
 *  0 - 1 - 2
 *   \     /
 *    - 3 -
 *      |
 *      4
 */
int NET[5][5] = {
    { 0, 1, 0, 1, 0 },
    { 1, 0, 1, 0, 0 },
    { 0, 1, 0, 1, 0 },
    { 1, 0, 1, 0, 1 },
    { 0, 0, 0, 1, 0 }
};

int main (int argc, char** argv) {
    for (int i=0; i<5; i++) {
        for (int j=0; j<5; j++) {
            assert(NET[i][j] == NET[j][i]);
        }
    }

    assert(argc == 3);
    char me   = atoi(argv[1]);
    int  port = atoi(argv[2]);

    srand(me);
    p2p_init(me,port);

    sleep(1);
    for (int i=me+1; i<5; i++) {
        if (NET[(int)me][i]) {
            p2p_link("localhost", 5000+i, i);
        }
    }

    while (1) {
        usleep(10);

        uint8_t n;
        p2p_evt evt;
        while (p2p_step(&n, &evt)) {
            printf("<<< [%d] %d/%d\n", me, evt.id, evt.pay.i1);
        }

        if (rand()%20000 == 0) {
            static int i = 1;
printf(">>>>>> [%d] %d/%d\n", me, me, i);
            p2p_evt evt = { me, {.i1=i++} };
            p2p_bcast(0, 1, &evt);
        }
#if 1
        if (rand()%50000 == 0) {
printf("-=-=-=-=- dump from %d\n", me);
p2p_dump();
puts("-=-=-=-=-");
        }
#endif
    }
    p2p_quit();
    return 0;
}
