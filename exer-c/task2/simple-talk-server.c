#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 10130

int main(int argc, char** argv) {
    /* ソケットの生成 */
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }
    /* ソケットアドレス再利用の設定 */
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    /* client 受付用ソケットの情報設定 */
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    /* ソケットにソケットアドレスを割り当てる */
    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("bind");
        exit(1);
    }
    /* 待ち受けクライアント数の設定 */
    if (listen(server_socket, 5) < 0) {
        perror("listen");
        exit(1);
    }
    do {
        int nbytes = 256;
        /* クライアントの受付 */
        int client_socket;
        struct sockaddr_in client;
        int clen = sizeof(client);
        if ((client_socket = accept(server_socket, (struct sockaddr*)&client, &clen)) < 0) {
            perror("accept");
            exit(2);
        }
        /* クライアントのホスト情報の取得 */
        struct hostent* cp;
        cp = gethostbyaddr((char*)&client.sin_addr, sizeof(struct in_addr), AF_INET);
        printf("connected\n");
        /* select()の準備 */
        fd_set rfds;
        struct timeval tv;
        /* select()が監視する時間設定 */
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        do {
            /* select()が監視する対象 */
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);
            FD_SET(client_socket, &rfds);
            /* stdio client_socketが受付可能か判断 */
            if (select(client_socket + 1, &rfds, NULL, NULL, &tv) > 0) {
                /* stdin */
                if (FD_ISSET(0, &rfds)) {
                    /* 送信文字列受け取り */
                    char str[256] = "";
                    if (scanf("%255[^\n]%*[^\n]", str) == EOF) break;
                    scanf("%*c");
                    /* serverに送信 */
                    nbytes = write(client_socket, str, sizeof(str));
                }
                /* client_socket */
                if (FD_ISSET(client_socket, &rfds)) {
                    char str[256] = "";
                    if ((nbytes = read(client_socket, str, sizeof(str))) < 0) {
                        perror("read");
                        break;
                    }
                    else if (nbytes == 0) break;
                    printf("%s\n", str);
                }
            }
        } while (nbytes != 0);
        close(client_socket);
        printf("closed\n");
    } while (1);
}