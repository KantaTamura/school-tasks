#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 10120

int main(int argc, char** argv) {
    /* 入力エラー */
    if (argc != 2) {
        fprintf(stderr, "no command line args\n");
        exit(-1);
    }
    /* ソケットの生成 */
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }
    /* serverの情報設定 */
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    struct hostent* hp;
    if ((hp = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "unknown server %s\n", argv[1]);
        exit(1);
    }
    bcopy(hp->h_addr_list[0], &server.sin_addr, hp->h_length);
    /* serverとの接続 */
    if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "can't connect %s\n", argv[1]);
        exit(-1);
    }
    printf("connected\n");
    /* 処理 */
    for (;;) {
        /* 送信文字列受け取り */
        char str[256] = "";
        if (scanf("%255[^\n]%*[^\n]", str) == EOF) break;
        scanf("%*c");
        /* serverに送信 */
        int nbytes;
        nbytes = write(sock, str, sizeof(str));
        /* serverからの返信受け取り */
        char buf[256] = "";
        if ((nbytes = read(sock, buf, nbytes)) < 0) {
            perror("read");
            break;
        }
        printf("%s\n", buf);
    }
    close(sock);
    return 0;
}