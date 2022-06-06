#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 256
#define TIMEOUT 10

void myalarm(int sec) {
    static int pid[2] = { 0, -1 };
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
    //alarm(sec);
}

void timeout() {
    printf("This program is timeout.\n");
    exit(0);
}

int main() {
    char buf[BUFSIZE];

    if (signal(SIGALRM, timeout) == SIG_ERR) {
        perror("signal failed.");
        exit(1);
    }
    myalarm(TIMEOUT);
    while (fgets(buf, BUFSIZE, stdin) != NULL) {
        printf("echo: %s", buf);
        myalarm(TIMEOUT);
    }
}