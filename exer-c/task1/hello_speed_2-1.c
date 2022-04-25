#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define COUNT 100

int main(int ac, char* av[]) {
    int i;
    char message[] = "hello";
    for (i = 0; i < COUNT; i++) {
        write(0, message, strlen(message));
    }
    return 0;
}