#!/bin/sh

valgrind ./xmain 0 5000 &
valgrind ./xmain 1 5001 &
valgrind ./xmain 2 5002 &
valgrind ./xmain 3 5003 &
valgrind ./xmain 4 5004 &

wait
