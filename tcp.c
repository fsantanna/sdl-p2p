void tcp_recv_n (TCPsocket s, int N, char* buf) {
    int i = 0;
    while (i < N) {
        i += SDLNet_TCP_Recv(s, &buf[i], N-i);
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

void tcp_send_u8 (TCPsocket s, uint8_t v) {
    assert(SDLNet_TCP_Send(s, &v, sizeof(v)) == sizeof(v));
}

void tcp_send_u32 (TCPsocket s, uint32_t v) {
    v = htobe32(v);
    assert(SDLNet_TCP_Send(s, &v, sizeof(v)) == sizeof(v));
}


