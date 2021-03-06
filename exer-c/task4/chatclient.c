#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>

#define port 10140

void exit_socket(int socket);
void nl_last_char(char* str);
void null_last_char(char* str);

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s hostname username\n", argv[0]);
        exit(-1);
    }

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
        exit_socket(server_socket);
    }
    memcpy(&server.sin_addr, hp->h_addr_list[0], hp->h_length);

    if (connect(server_socket, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "can't connect %s\n", argv[1]);
        exit_socket(server_socket);
    }

    char connect_str[126];
    if (read(server_socket, connect_str, sizeof(connect_str)) < 0) {
        perror("read");
        exit_socket(server_socket);
    }
    if (strncmp(connect_str, "REQUEST ACCEPTED\n", sizeof("REQUEST ACCEPTED\n") - 1) != 0) {
        fprintf(stderr, "request rejected!\n");
        exit_socket(server_socket);
    }

    char username[126];
    strcpy(username, argv[2]);
    nl_last_char(username);
    write(server_socket, username, sizeof(username));

    char regist_str[126];
    if (read(server_socket, regist_str, sizeof(regist_str)) < 0) {
        perror("read");
        exit_socket(server_socket);
    }
    if (strncmp(regist_str, "USERNAME REGISTERED\n", sizeof("USERNAME REGISTERED\n") - 1) != 0) {
        null_last_char(username);
        fprintf(stderr, "%s is registered!\n", username);
        exit_socket(server_socket);
    }

    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    for (;;) {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(server_socket, &rfds);

        if (select(server_socket + 1, &rfds, NULL, NULL, &tv) > 0) {
            if (FD_ISSET(0, &rfds)) {
                char str[512] = "";
                if (scanf("%511[^\n]%*[^\n]", str) == EOF) break;
                scanf("%*c");
                printf("\e[A\e[K");
                write(server_socket, str, sizeof(str));
            }
            if (FD_ISSET(server_socket, &rfds)) {
                char str[1024] = "";
                int nbytes;
                if ((nbytes = read(server_socket, str, sizeof(str))) < 0) {
                    perror("read");
                    break;
                }
                else if (nbytes == 0) break;
                printf("%s\n", str);
            }
        }
    }
    close(server_socket);
}

void exit_socket(int socket) {
    close(socket);
    exit(-1);
}

void nl_last_char(char* str) {
    for (int i = 0;;i++)
        if (str[i] == '\0') {
            *(str + i) = '\n';
            return;
        }
}

void null_last_char(char* str) {
    for (int i = 0;;i++)
        if (str[i] == '\n') {
            *(str + i) = '\0';
            return;
        }
}