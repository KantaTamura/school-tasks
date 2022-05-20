#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PORT 10000

int main(void) {
    int sock;
    struct sockaddr_in svr;
    struct sockaddr_in sender;
    int sender_len, len;
    struct hostent* hp;
    char buf[1024], senderstr[1024];

    /* ソケットの生成 */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    /* client 受付用ソケットの情報設定 */
    bzero(&svr, sizeof(svr));
    svr.sin_family = AF_INET;
    svr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr.sin_port = htons(PORT);
    /* ソケットにソケットアドレスを割り当てる */
    if (bind(sock, (struct sockaddr*)&svr, sizeof(svr)) < 0) {
        perror("bind");
        exit(1);
    }
    /* パケットの受信と送信 */
    while (1) {
        bzero(&sender, sizeof(sender));
        sender_len = sizeof(sender);
        if ((len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&sender, &sender_len)) < 0) {
            perror("recvfrom");
            exit(1);
        }
        inet_ntop(AF_INET, &sender.sin_addr, senderstr, sizeof(senderstr));
        write(1, buf, len);
        printf(" from [%s:%d]\n", senderstr, sender.sin_port);
    }
}