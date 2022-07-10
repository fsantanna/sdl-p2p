void tcp_recv_n (TCPsocket s, int n, char* buf) {
    int i = 0;
    while (i < n) {
        i += SDLNet_TCP_Recv(s, &buf[i], n-i);
    }
}

uint8_t tcp_recv_u8 (TCPsocket s) {
    uint8_t v;
    tcp_recv_n(s, sizeof(v), (char*)&v);
    return v;
}

uint32_t tcp_recv_u32 (TCPsocket s) {
    uint32_t v;
    tcp_recv_n(s, sizeof(v), (char*)&v);
    return be32toh(v);
}

void tcp_send_n (TCPsocket s, int n, char* buf) {
    assert(SDLNet_TCP_Send(s,buf,n) == n);
    //int x = SDLNet_TCP_Send(s,buf,n);
    //printf(">>> %d, %s\n", x, SDLNet_GetError());
    //assert(x == n);
}

void tcp_send_u8 (TCPsocket s, uint8_t v) {
    tcp_send_n(s, sizeof(v), (char*) &v);
}

void tcp_send_u32 (TCPsocket s, uint32_t v) {
    v = htobe32(v);
    tcp_send_n(s, sizeof(v), (char*) &v);
}
