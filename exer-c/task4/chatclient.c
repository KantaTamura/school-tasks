#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>

const int port = 10140;

bool starts_with(char* p, char* q);

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s hostname username\n", argv[0]);
        exit(-1);
    }
    // TODO: delete
    printf("hostname : %s, username : %s\n", argv[1], argv[2]);
    
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    struct hostent* hp;
    if ((hp = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "unknown server %s\n", argv[1]);
        exit(1);
    }
    memcpy(&server.sin_addr, hp->h_addr_list[0], hp->h_length);

    if (connect(server_socket, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "can't connect %s\n", argv[1]);
        exit(-1);
    }
    // TODO: delete
    printf("connected to %s\n", argv[1]);

    char connect_str[126];
    if (read(server_socket, connect_str, sizeof(connect_str)) < 0) {
        perror("read");
        exit(-1);
    }
    if (!starts_with(connect_str, "REQUEST ACCEPTED\n")) {
        fprintf(stderr, "not REQUEST ACCEPTED\n");
        exit(-1);
    }
}

bool starts_with(char* p, char* q) {
    return strncmp(p, q, sizeof(q)) == 0;
}