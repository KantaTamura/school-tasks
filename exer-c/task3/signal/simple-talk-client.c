#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define PORT 10130
#define TIMEOUT 10

int timeout_flag = 0;

void wait_process() {
    int status;
    wait(&status);
}

void myalarm(int sec) {
    static int pid[2] = { 0, -1 };
    if (signal(SIGCHLD, wait_process) == SIG_ERR) {
        perror("signal failed.");
        exit(1);
    }
    if ((pid[0] = fork()) == -1) {
        perror("fork failed.");
        exit(1);
    }
    if (pid[0] == 0) {
        int parent_pid = getppid();
        sleep(10);
        if (kill(parent_pid, SIGALRM) == -1) {
            perror("kill failed.");
            exit(1);
        }
        exit(1);
    }
    else {
        if (pid[1] != -1) if (kill(pid[1], SIGTERM) == -1) {
            perror("kill failed.");
            exit(1);
        }
        pid[1] = pid[0];
    }
}

void timeout() {
    timeout_flag = 1;
}

int main(int argc, char** argv) {
    /* 入力エラー */
    if (argc != 2) {
        fprintf(stderr, "no command line args\n");
        exit(-1);
    }
    /* ソケットの生成 */
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
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
    if (connect(server_socket, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "can't connect %s\n", argv[1]);
        exit(-1);
    }
    printf("connected\n");
    /* select()の準備 */
    fd_set rfds;
    struct timeval tv;
    /* select()が監視する時間設定 */
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    /* */
    if (signal(SIGALRM, timeout) == SIG_ERR) {
        perror("signal failed.");
        exit(1);
    }
    myalarm(TIMEOUT);
    /* 処理 */
    for (;;) {
        /* select()が監視する対象 */
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(server_socket, &rfds);
        /* */
        if (timeout_flag == 1) {
            printf("timeout\n");
            break;
        }
        /* stdio client_socketが受付可能か判断 */
        if (select(server_socket + 1, &rfds, NULL, NULL, &tv) > 0) {
            /* stdin */
            if (FD_ISSET(0, &rfds)) {
                /* 送信文字列受け取り */
                char str[256] = "";
                if (scanf("%255[^\n]%*[^\n]", str) == EOF) break;
                scanf("%*c");
                /* serverに送信 */
                write(server_socket, str, sizeof(str));
            }
            /* client_socket */
            if (FD_ISSET(server_socket, &rfds)) {
                char str[256] = "";
                int nbytes;
                if ((nbytes = read(server_socket, str, sizeof(str))) < 0) {
                    perror("read");
                    break;
                }
                else if (nbytes == 0) break;
                printf("%s\n", str);
            }
            myalarm(TIMEOUT);
        }
    }
    close(server_socket);
    return 0;
}