#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 256

int main(int argc, char** argv) {
    char buf[BUFSIZE];
    int read_parent[2], read_child[2];
    int pid, msglen, status;
    setbuf(stdout, NULL);
    if (argc != 3) {
        printf("bad argument.\n");
        exit(1);
    }
    if (pipe(read_parent) == -1) {
        perror("read_parent pipe failed.");
        exit(1);
    }
    if (pipe(read_child) == -1) {
        perror("read_child pipe failed.");
        exit(1);
    }
    if ((pid = fork()) == -1) {
        perror("fork failed.");
        exit(1);
    }
    // Child process
    if (pid == 0) {
        // write
        close(read_parent[0]);
        msglen = strlen(argv[1]) + 1;
        if (write(read_parent[1], argv[1], msglen) == -1) {
            perror("read_parent pipe write.");
            exit(1);
        }
        // read
        close(read_child[1]);
        if (read(read_child[0], buf, BUFSIZE) == -1) {
            perror("read_child pipe read.");
            exit(1);
        }
        printf("Message from parent process: \n");
        printf("\t%s\n", buf);
        exit(0);
    }
    // Parent process
    else {
        // write
        close(read_child[0]);
        msglen = strlen(argv[2]) + 1;
        if (write(read_child[1], argv[2], msglen) == -1) {
            perror("read_child pipe write.");
            exit(1);
        }
        // read
        close(read_parent[1]);
        if (read(read_parent[0], buf, BUFSIZE) == -1) {
            perror("read_parent pipe read.");
            exit(1);
        }
        printf("Message from child process: \n");
        printf("\t%s\n", buf);
        wait(&status);
    }
}