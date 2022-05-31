#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int sock, csock;
    struct sockaddr_in svr;
    struct sockaddr_in clt;
    struct hostent* cp;
    int clen, nbytes, reuse;
    char rbuf[1024];
    /* ソケットの生成 */
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }
    /* ソケットアドレス再利用の設定 */
    reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    /* client 受付用ソケットの情報設定 */
    bzero(&svr, sizeof(svr));
    svr.sin_family = AF_INET;
    svr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr.sin_port = htons(10120);
    /* ソケットにソケットアドレスを割り当てる */
    if (bind(sock, (struct sockaddr*)&svr, sizeof(svr)) < 0) {
        perror("bind");
        exit(1);
    }
    /* 待ち受けクライアント数の設定 */
    if (listen(sock, 5) < 0) {
        perror("listen");
        exit(1);
    }
    do {
        /* クライアントの受付 */
        clen = sizeof(clt);
        if ((csock = accept(sock, (struct sockaddr*)&clt, &clen)) < 0) {
            perror("accept");
            exit(2);
        }
        /* クライアントのホスト情報の取得 */
        cp = gethostbyaddr((char*)&clt.sin_addr, sizeof(struct in_addr), AF_INET);
        printf("[%s]\n", cp->h_name);
        do {
            /* クライアントからのメッセージ受信 */
            if ((nbytes = read(csock, rbuf, sizeof(rbuf))) < 0) {
                perror("read");
            }
            else {
                for (int i = 0; i < sizeof(rbuf); i++) {
                    if (rbuf[i] >= 'a' && rbuf[i] <= 'z') rbuf[i] -= 'a' - 'A';
                }
                write(csock, rbuf, nbytes);
            }
        } while (nbytes != 0);
        close(csock);
        printf("closed\n");
    } while (1);
}