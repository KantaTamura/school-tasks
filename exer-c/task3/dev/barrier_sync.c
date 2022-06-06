#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define NUMPROCS 4
#define FILE_LINE 10

char filename[256] = "file";

void make_file_name(char* file, int num) {
    strcpy(file, filename);
    char add_file_name[128] = { '-', num + '0' };
    strcat(file, add_file_name);
}

void file_write(int num) {
    char file[256];
    make_file_name(file, num);
    FILE* fp;
    if ((fp = fopen(file, "w")) == NULL) exit(1);
    for (int i = 0; i < FILE_LINE; i++) fprintf(fp, "%d\n", rand());
}

void file_read(int num, int line[FILE_LINE]) {
    char file[256];
    make_file_name(file, num);
    FILE* fp;
    if ((fp = fopen(file, "r")) == NULL) exit(1);
    for (int i = 0; i < FILE_LINE; i++) fscanf(fp, "%d\n", line[i]);
}

int main() {
    int pid, status;
    // set stdout to be unbufferd
    setbuf(stdout, NULL);

    srand(getpid());

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
    semctl(sid, 0, SETVAL, NUMPROCS);

    for (int i = 0; i < NUMPROCS; i++) {
        if ((pid = fork()) == -1) {
            perror("fork failed.");
            exit(1);
        }
        // Child process
        if (pid == 0) {
            struct sembuf stop_sem = { 0, 0, 0 }, sub_sem = { 0 , -1, 0 };
            file_write(i);
            printf("Process-%d:stop\n", i);
            if (semop(sid, &sub_sem, 1) == -1) exit(1);
            if (semop(sid, &stop_sem, 1) == -1) exit(1);
            printf("Process-%d:restart\n", i);
            int line[FILE_LINE];
            file_read(i, line);
            printf("Process-%d:success\n", i);
            exit(1);
        }
    }
    for (int i = 0; i < NUMPROCS; i++) {
        wait(&status);
    }

    semctl(sid, 0, IPC_RMID);
    exit(0);
}