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


