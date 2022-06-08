#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUMPROCS 4

char filename[] = "counter";

int critical_section() {
    FILE* ct;
    int count;
    if ((ct = fopen(filename, "r")) == NULL) exit(1);
    fscanf(ct, "%d\n", &count);
    count++;
    fclose(ct);
    if ((ct = fopen(filename, "w")) == NULL) exit(1);
    fprintf(ct, "%d\n", count);
    fclose(ct);
    return count;
}

int main() {
    int pid, status;
    // set stdout to be unbufferd
    setbuf(stdout, NULL);

    // Set the initial value
    FILE* ct;
    if ((ct = fopen(filename, "w")) == NULL) exit(1);
    int count = 0;
    fprintf(ct, "%d\n", count);
    fclose(ct);

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
    for (int i = 1; i < NUMPROCS; i++) semctl(sid, i, SETVAL, 1);

    for (int i = 0; i < NUMPROCS; i++) {
        if ((pid = fork()) == -1) {
            perror("fork failed.");
            exit(1);
        }
        // Child process
        if (pid == 0) {
            struct sembuf stop_sem = { i, 0, 0 }, sub_sem = { i + 1 , -1, 0 };
            if (semop(sid, &stop_sem, 1) == -1) exit(1);
            count = critical_section();
            if (i < NUMPROCS - 1) if (semop(sid, &sub_sem, 1) == -1) exit(1);
            printf("count = %d\n", count);
            exit(0);
        }
    }
    for (int i = 0; i < NUMPROCS; i++) {
        wait(&status);
    }
    semctl(sid, 0, IPC_RMID);
    exit(0);
}