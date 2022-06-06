#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pid, status;

    int A[100], B[100];
    for (int i = 0; i < 100; i++) {
        A[i] = i;
        B[(100 - 1) - i] = i;
    }

    // set stdout to be unbufferd
    setbuf(stdout, NULL);

    // Preparing the semaphore
    key_t key;
    if ((key = ftok(".", 1)) == -1) {
        fprintf(stderr, "ftok path does not exist.\n");
        exit(1);
    }
    int sid;
    if ((sid = semget(key, 4, 0666 | IPC_CREAT)) == -1) {
        perror("semget error.");
        exit(1);
    }
    semctl(sid, 1, SETVAL, 1);

    if ((pid = fork()) == -1) {
        perror("fork failed.");
        exit(1);
    }
    // Child process
    if (pid == 0) {
        struct sembuf stop_sem = { 1, 0, 0 }, add_sem = { 1, 1, 0 }, sub_sem = { 0 , -1, 0 };
        for (int i = 0; i < 100; i++) {
            if (semop(sid, &stop_sem, 1) == -1) exit(1);
            printf("B[%d]:%d\n", i, B[i]);
            if (semop(sid, &add_sem, 1) == -1) exit(1);
            if (semop(sid, &sub_sem, 1) == -1) exit(1);
        }
        exit(0);
    }
    else {
        struct sembuf stop_sem = { 0, 0, 0 }, add_sem = { 0, 1, 0 }, sub_sem = { 1 , -1, 0 };
        for (int i = 0; i < 100; i++) {
            if (semop(sid, &stop_sem, 1) == -1) exit(1);
            printf("A[%d]:%d\t", i, A[i]);
            if (semop(sid, &add_sem, 1) == -1) exit(1);
            if (semop(sid, &sub_sem, 1) == -1) exit(1);
        }
        wait(&status);
    }
    semctl(sid, 0, IPC_RMID);
    exit(0);
}