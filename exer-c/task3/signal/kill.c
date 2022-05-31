#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUMPROCS 3

int main() {
    int pid, status;
    int children[NUMPROCS];
    for (int i = 0; i < NUMPROCS; i++) {
        if ((pid = fork()) == -1) {
            perror("fork failed.");
            exit(1);
        }
        if (pid == 0) {
            while (1) {
                printf("Child process: i = %d\n", i);
                sleep(1);
            }
            exit(1);
        }
        else {
            children[i] = pid;
        }
    }
    for (int i = 0; i < NUMPROCS; i++) {
        sleep(3);
        if (kill(children[i], SIGTERM) == -1) {
            perror("kill failed.");
            exit(1);
        }
    }
    printf("Parent process: all children are terminated.\n");
    for (int i = 0; i < NUMPROCS; i++) wait(&status);
}