#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s hostname username\n", argv[0]);
        exit(-1);
    }
    printf("hostname : %s, username : %s\n", argv[1], argv[2]);
}