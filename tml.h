#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))

enum {
    TML_RET_NONE = 0,
    TML_RET_QUIT,
    TML_RET_REC,
    TML_RET_TRV
};

enum {
    TML_EVT_INIT = 0,
    TML_EVT_TICK,
    TML_EVT_NEXT
};

typedef struct {
    uint8_t id;
    union {
        int tick;
        int i1;
        struct {
            int _1,_2;
        } i2;
        struct {
            int _1,_2,_3;
        } i3;
        struct {
            int _1,_2,_3,_4;
        } i4;
    } pay;
} tml_evt;

void tml_loop (
    int fps,                                // desired frame rate
    int mem_n,                              // memory size in bytes
    void* mem_buf,                          // pointer to memory contents
    void (*cb_sim) (tml_evt),               // simulation callback
    void (*cb_eff) (int trv),               // effects callback
    int (*cb_rec) (SDL_Event*,tml_evt*),    // event recording callback
    int (*cb_trv) (SDL_Event*,int,int,int*) // travel mode callback
);
