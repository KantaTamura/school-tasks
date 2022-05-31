#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUMPROCS 3

int main() {
    int pid, status, c_pid;
    // set stdout to be unbufferd
    setbuf(stdout, NULL);
    for (int i = 0; i < NUMPROCS; i++) {
        if ((pid = fork()) == -1) {
            perror("fork failed.");
            exit(1);
        }
        // Child process
        if (pid == 0) {
            sleep(1);
            printf("Child process i=%d\n", i);
            exit(0);
        }
    }
    // Parent process
    for (int i = 0; i < NUMPROCS; i++) {
        c_pid = wait(&status);
        printf("Parent process: child (%d)\n", c_pid);
    }
}
