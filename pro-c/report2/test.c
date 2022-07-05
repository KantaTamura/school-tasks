#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

bool does_exist_directory(char*);
void path_reflesh(char*);
bool starts_with(char*, char*);

int main(int argc, char** argv) {
    // printf("%d\n", system("gcc >/dev/null 2>&1"));
    char str[1024] = "/home/i0ta/code//school/././././//pro-c/../../../";
    printf("%s\n", does_exist_directory(str) ? "exist!" : "no...");
    printf("%s\n", str);
}

bool does_exist_directory(char* path) {
    path_reflesh(path);
    struct dirent* dir;
    if (opendir(path) == NULL) { perror(path); return false; }
    return true;
}

void path_reflesh(char* dir) {
    int now_iter = 0, read_iter = 0;
    char path[1024];
    for (; read_iter < strlen(dir); now_iter++) {
        for (;;) {
            bool is_ok = true;
            if (starts_with(dir + read_iter, " ")) { read_iter += 1; is_ok = false; printf("no\n"); }
            if (starts_with(dir + read_iter, "//")) if (!starts_with(path + now_iter - 1, "/")) { read_iter += 1; is_ok = false; }
            else { read_iter += 2; is_ok = false; }
            if (starts_with(dir + read_iter, "./")) if (!starts_with(dir + read_iter - 1, ".")) { read_iter += 2; is_ok = false; }
            if (starts_with(dir + read_iter, "../")) for (int l = 2; l <= now_iter; l++) if (starts_with(path + now_iter - l, "/")) {
                read_iter += 3; now_iter -= l - 1; is_ok = false; break;
            }
            if (is_ok) break;
        }
        path[now_iter] = dir[read_iter];
        read_iter++;
    }
    path[now_iter] = '\0';
    strcpy(dir, path);
}

bool starts_with(char* p, char* q) {
    return strncmp(p, q, strlen(q)) == 0;
}