# SDL - P2P

`sdl-p2p` is a peer-to-peer library for SDL.
Events are broadcast between peers transparently.

## Example

```
$ chmod +x main.c
$ ./main.sh
```

## API

- `void p2p_init (uint8_t me, int port)`
    - A peer needs a unique identifier and a listening port.
- `void p2p_link (char* host, int port, uint8_t me)`
    - A two-way link must be explicitly created from one of the ends.
- `void p2p_bcast (p2p_evt* evt)`
    - A peer can broadcast events, which are automatically received by other
      peers (including itself).
- `int p2p_step (p2p_evt* evt)`
    - A peer must `step` to check the network, which returns broadcast events.
- `void p2p_quit (void)`

An event has the following structure:

```
typedef struct {
    uint8_t id;         // user-defined identifier
    uint8_t n;          // number of ints in the payload
    union {
        int i1;             // 1-int payload
        struct {
            int _1,_2;
        } i2;               // 2-int payload
        struct {
            int _1,_2,_3;
        } i3;               // 3-int payload
        struct {
            int _1,_2,_3,_4;
        } i4;               // 4-int payload
    } pay;
} p2p_evt;
```
