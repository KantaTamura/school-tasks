#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef enum CommandStatus {
  foreground,
  background,
  none,
} CommandStatus;

typedef struct Command {
  char argument[1024];
  void (*exe_command)();
  CommandStatus status;
} Command;

typedef struct Ailias {
  char before[1024];
  char after[1024];
} Ailias;

typedef struct ShellInfo {
  char user_name[256];
  char current_dir[1024];
  char prompt[1024];
  Command current_command;
  Command buffer_command[32];
  int buffer_command_num;
  char dir_stack[1024][1024];
  int dir_stack_num;
  Ailias ailias[1024];
  int ailias_num;
} ShellInfo;

bool check_arg(char* arg, char* cmd_name);
Command before_command(ShellInfo* shell, char* str);
Command new_command(char* argument, void (*exe_command)(ShellInfo*), CommandStatus status);
Command none_command();
ShellInfo new_shell_info();

void change_str_alias(ShellInfo* shell, char* str);
void change_str_ast(ShellInfo* shell, char* str);

void buf_command(ShellInfo* shell);
void cpy_command(Command* a, Command* b);

void wait_process() {
  int status;
  wait(&status);
}

void exit_shell();

bool starts_with(char* p, char* q);
Command parse_command(ShellInfo* shell, char* str);
void func_exit(ShellInfo* shell);
void func_cd(ShellInfo* shell);
void func_ls(ShellInfo* shell);
void func_pushd(ShellInfo* shell);
void func_dirs(ShellInfo* shell);
void func_popd(ShellInfo* shell);
void func_history(ShellInfo* shell);
void func_prompt(ShellInfo* shell);
void func_alias(ShellInfo* shell);
void func_unalias(ShellInfo* shell);
void func_cat(ShellInfo* shell);
void func_echo(ShellInfo* shell);
void func_wc(ShellInfo* shell);
void func_pwd(ShellInfo* shell);
void execute_command(ShellInfo* shell);

bool path_exist(ShellInfo* shell, char* path);
int split_blank(char** args, char* arg);

bool does_exist_directory(char* path);
void path_reflesh(char* dir);

int main() {
  ShellInfo main_shell = new_shell_info();

  setbuf(stdout, NULL);

  if (signal(SIGCHLD, wait_process) == SIG_ERR) {
    perror("signal failed.");
    exit(1);
  }

  for (;;) {
    char input_string_buffer[1024] = "";

    printf("%s", main_shell.prompt);
    if (scanf("%1023[^\n]%*[^\n]", input_string_buffer) == EOF) { printf("\n"); exit_shell(); }
    scanf("%*c");

    main_shell.current_command = parse_command(&main_shell, input_string_buffer);

    switch (main_shell.current_command.status) {
    case foreground:
    case background:
      main_shell.current_command.exe_command(&main_shell);
      buf_command(&main_shell);
      break;
    case none:
      break;
    }
    printf("\n");
  }
}

Command parse_command(ShellInfo* shell, char* str) {
  CommandStatus status = foreground;
  for (;;str += 1) if (*str != ' ') break;
  change_str_ast(shell, str);
  if (starts_with(str, "alias"))    return new_command(str, func_alias, foreground);
  if (starts_with(str, "unalias"))  return new_command(str, func_unalias, foreground);
  change_str_alias(shell, str);
  if (*str == '\0') return none_command();
  if (starts_with(str, "exit"))     return new_command(str, func_exit, foreground);
  if (starts_with(str, "cd"))       return new_command(str, func_cd, foreground);
  if (starts_with(str, "ls"))       return new_command(str, func_ls, foreground);
  if (starts_with(str, "pushd"))    return new_command(str, func_pushd, foreground);
  if (starts_with(str, "dirs"))     return new_command(str, func_dirs, foreground);
  if (starts_with(str, "popd"))     return new_command(str, func_popd, foreground);
  if (starts_with(str, "history"))  return new_command(str, func_history, foreground);
  if (starts_with(str, "!"))        return before_command(shell, str);
  if (starts_with(str, "prompt"))   return new_command(str, func_prompt, foreground);
  if (starts_with(str, "cat"))      return new_command(str, func_cat, foreground);
  if (starts_with(str, "echo"))     return new_command(str, func_echo, foreground);
  if (starts_with(str, "wc"))       return new_command(str, func_wc, foreground);
  if (starts_with(str, "pwd"))      return new_command(str, func_pwd, foreground);
  int iter = strlen(str) - 1; for (;iter >= 0;) if (str[iter] == ' ') iter--; else break;
  if (str[iter] == '&') { str[iter] = '\0'; status = background; }
  return new_command(str, execute_command, status);
}

void change_str_alias(ShellInfo* shell, char* str) {
  for (int iter = 0; iter < strlen(str); iter++) {
    for (int a_n = 0; a_n < shell->ailias_num; a_n++)
      if (starts_with(str + iter, shell->ailias[a_n].after)) {
        char buf[1024];
        int start = iter, end = iter + strlen(shell->ailias[a_n].after);
        strncat(buf, str, start); buf[start] = '\0';
        strcat(buf, shell->ailias[a_n].before);
        strcat(buf, str + end);
        iter += strlen(shell->ailias[a_n].before) - 1;
        strcpy(str, buf);
      }
  }
}

void change_str_ast(ShellInfo* shell, char* str) {
  for (int iter = 0; iter < strlen(str); iter++) {
    if (starts_with(str + iter, "*")) {
      char buf[1024] = "", ast[1024] = " ";
      int start = iter, end = iter + strlen("*");
      DIR* dir;
      struct dirent* dp;
      if ((dir = opendir(shell->current_dir)) == NULL) { perror(shell->current_dir); continue; }
      for (dp = readdir(dir);dp != NULL;dp = readdir(dir))
        if (dp->d_type == 8) {
          strcat(ast, dp->d_name);
          strcat(ast, " ");
        }
      closedir(dir);
      strncat(buf, str, start); buf[start] = '\0';
      strcat(buf, ast);
      strcat(buf, str + end);
      iter += strlen(ast) - 1;
      strcpy(str, buf);
    }
  }
}

Command before_command(ShellInfo* shell, char* str) {
  if (shell->buffer_command_num == 0) { printf("Error : no command has been executed so far\n"); return none_command(); }
  char search[1024];
  strcpy(search, str + 1);
  if (search[0] == '\0') { printf("Error : requier !<str>\n"); return none_command(); }
  if (strcmp(search, "!") == 0) return shell->buffer_command[shell->buffer_command_num - 1];
  for (int i = shell->buffer_command_num - 1; i >= 0; i--)
    if (starts_with(shell->buffer_command[i].argument, search)) return shell->buffer_command[i];
  printf("Error : no match command\n");
  return none_command();
}

bool check_arg(char* arg, char* cmd_name) {
  int iter = 0;
  char str[1024];
  strcpy(str, arg);
  iter += strlen(cmd_name);
  for (;;) if (str[iter] == ' ') iter++; else break;
  if (str[iter] != '\0') return false;
  return true;
}

Command new_command(char* argument, void (*exe_command)(ShellInfo*), CommandStatus status) {
  Command command;
  strcpy(command.argument, argument);
  command.exe_command = exe_command;
  command.status = status;
  return command;
}

Command none_command() {
  Command command;
  command.status = none;
  return command;
}

ShellInfo new_shell_info() {
  ShellInfo shell;
  strcpy(shell.user_name, getenv("USER"));
  strcpy(shell.current_dir, getenv("HOME"));
  strcpy(shell.prompt, "Command: ");
  shell.buffer_command_num = 0;
  shell.dir_stack_num = 0;
  shell.ailias_num = 0;
  return shell;
}

void buf_command(ShellInfo* shell) {
  if (shell->buffer_command_num == 32) {
    for (int i = 1; i < 32; i++) cpy_command(&shell->buffer_command[i - 1], &shell->buffer_command[i]);
    shell->buffer_command_num--;
  }
  cpy_command(&shell->buffer_command[shell->buffer_command_num], &shell->current_command);
  shell->buffer_command_num++;
}

void cpy_command(Command* a, Command* b) {
  strcpy(a->argument, b->argument);
  a->exe_command = b->exe_command;
  a->status = b->status;
}

bool starts_with(char* p, char* q) {
  return strncmp(p, q, strlen(q)) == 0;
}

void exit_shell() {
  printf("exit shell");
  exit(0);
}

void func_exit(ShellInfo* shell) {
  if (!check_arg(shell->current_command.argument, "exit")) { printf("Error : There is a character after \"exit\"\n"); return; }
  exit_shell();
}

bool path_exist(ShellInfo* shell, char* path) {
  if (path[0] == '/' && !does_exist_directory(path)) return false;
  if (path[0] != '/' || !does_exist_directory(path)) {
    strcpy(path, shell->current_dir);
    strcat(path, "/");
    strcat(path, shell->current_command.argument + strlen("cd"));
    if (!does_exist_directory(path)) return false;
  }
  return true;
}

void func_cd(ShellInfo* shell) {
  char arg[1024]; int iter = strlen("cd");
  for (;;) if (shell->current_command.argument[iter] == ' ') iter++; else break;
  if (shell->current_command.argument[iter] == '\0') { strcpy(shell->current_dir, getenv("HOME")); return; }
  strcpy(arg, shell->current_command.argument + iter);
  if (path_exist(shell, arg)) strcpy(shell->current_dir, arg);
}

void func_ls(ShellInfo* shell) {
  char path[1024]; int iter = strlen("ls");
  for (;;) if (shell->current_command.argument[iter] == ' ') iter++; else break;
  strcpy(path, shell->current_command.argument + iter);
  if (path[0] != '\0') if (!path_exist(shell, path)) return;
  if (path[0] == '\0') strcpy(path, shell->current_dir);
  DIR* dir;
  struct dirent* dp;
  if ((dir = opendir(path)) == NULL) { perror(path); return; }
  for (dp = readdir(dir);dp != NULL;dp = readdir(dir)) {
    printf("\t%s\n", dp->d_name);
  }
  closedir(dir);
}

void func_pushd(ShellInfo* shell) {
  if (!check_arg(shell->current_command.argument, "pushd")) { printf("Error : There is a character after \"pushd\"\n"); return; }
  strcpy(shell->dir_stack[shell->dir_stack_num++], shell->current_dir);
}

void func_dirs(ShellInfo* shell) {
  if (!check_arg(shell->current_command.argument, "dirs")) { printf("Error : There is a character after \"dirs\"\n"); return; }
  if (shell->dir_stack_num == 0) { printf("Error : no item in directory stack\n"); return; }
  printf("directory stack\n");
  for (int i = shell->dir_stack_num - 1; i >= 0; i--)
    printf("\t%s\n", shell->dir_stack[i]);
}

void func_popd(ShellInfo* shell) {
  if (!check_arg(shell->current_command.argument, "popd")) { printf("Error : There is a character after \"popd\"\n"); return; }
  if (shell->dir_stack_num == 0) { printf("Error : no item in directory stack\n"); return; }
  strcpy(shell->current_dir, shell->dir_stack[--shell->dir_stack_num]);
}

void func_history(ShellInfo* shell) {
  if (!check_arg(shell->current_command.argument, "history")) { printf("Error : There is a character after \"history\"\n"); return; }
  if (shell->buffer_command_num == 0) { printf("Error : no command has been executed so far\n"); return; }
  printf("command history\n");
  for (int i = 0; i < shell->buffer_command_num; i++)
    printf("%d\t%s\n", i + 1, shell->buffer_command[i].argument);
}

void func_prompt(ShellInfo* shell) {
  int iter = strlen("prompt");
  for (;;) if (shell->current_command.argument[iter] == ' ') iter++; else break;
  if (shell->current_command.argument[iter] == '\0') { strcpy(shell->prompt, "Command: "); return; }
  strcpy(shell->prompt, shell->current_command.argument + iter);
}

void func_alias(ShellInfo* shell) {
  int iter = strlen("alias");
  char buf[1024];
  strcpy(buf, shell->current_command.argument);
  for (;;) if (buf[iter] == ' ') iter++; else break;
  if (buf[iter] == '\0') {
    if (shell->ailias_num == 0) { printf("Error : no item in alias list\n"); return; }
    for (int i = 0; i < shell->ailias_num; i++) printf("\t%s => %s\n", shell->ailias[i].after, shell->ailias[i].before);
    return;
  }
  char* commands[128];
  int num = split_blank(commands, buf);
  if (num != 3) { printf("Error: no argument\n"); return; }
  strcpy(shell->ailias[shell->ailias_num].before, commands[2]);
  strcpy(shell->ailias[shell->ailias_num].after, commands[1]);
  shell->ailias_num++;
}

void func_unalias(ShellInfo* shell) {
  if (shell->ailias_num == 0) { printf("Error : no item in alias list\n"); return; }
  int iter = strlen("unalias"), start = 0, end = 0;
  char buf[1024];
  strcpy(buf, shell->current_command.argument);
  char* commands[128];
  int num = split_blank(commands, buf);
  if (num != 2) { printf("Error: no argument\n"); return; }
  bool no_alias = true;
  for (int i = 0; i < shell->ailias_num; i++) {
    if (strcmp(commands[1], shell->ailias[i].after) == 0) {
      no_alias = false;
      for (int l = i + 1; l < shell->ailias_num; l++) {
        strcpy(shell->ailias[l - 1].after, shell->ailias[l].after);
        strcpy(shell->ailias[l - 1].before, shell->ailias[l].before);
      }
      shell->ailias_num--;
    }
  }
  if (no_alias) { printf("Error: no alias\n"); return; }
}

bool does_exist_file(char* path) {
  path_reflesh(path);
  struct stat statbuf;
  stat(path, &statbuf);
  if (S_ISREG(statbuf.st_mode)) return true;
  fprintf(stderr, "%s: no such file\n", path);
  return false;
}

bool file_exist(ShellInfo* shell, char* path) {
  char buf[1024]; strcpy(buf, path);
  if (path[0] == '/' && !does_exist_file(path)) return false;
  if (path[0] != '/' || !does_exist_file(path)) {
    strcpy(path, shell->current_dir);
    strcat(path, "/");
    strcat(path, buf);
    if (!does_exist_file(path)) return false;
  }
  return true;
}

void func_cat(ShellInfo* shell) {
  char path[1024]; int iter = strlen("cat");
  for (;;) if (shell->current_command.argument[iter] == ' ') iter++; else break;
  strcpy(path, shell->current_command.argument + iter);
  if (path[0] != '\0') if (!file_exist(shell, path)) return;
  if (path[0] == '\0') {
    char buf[1024]; ssize_t nread;
    while ((nread = read(0, buf, sizeof(buf))) > 0) write(0, buf, nread);
    return;
  }
  FILE* fp;
  if ((fp = fopen(path, "r")) == NULL) { perror(path); return; }
  for (;;) {
    char buf[1024];
    if (fscanf(fp, "%1023[^\n]%*[^\n]", buf) == EOF) break;
    fscanf(fp, "%*c");
    printf("%s\n", buf);
  }
  fclose(fp);
}

void func_echo(ShellInfo* shell) {
  char arg[1024], * args[128]; strcpy(arg, shell->current_command.argument + strlen("echo"));
  int count = split_blank(args, arg);
  for (int i = 0; i < count; i++) printf("%s ", args[i]);
  printf("\n");
}

void wc(FILE* fp, char* filename, int* nlines, int* nwords, int* nbytes) {
  char buf[1024];
  bool inword = false;
  int nl = 0, nw = 0, nb = 0;
  size_t nread;
  while (1) {
    if (filename)  if ((nread = fread(buf, 1, sizeof(buf), fp)) <= 0) break;
    if (!filename) if ((nread = read(0, buf, sizeof(buf))) <= 0) break;
    nb += nread;
    for (size_t i = 0; i < nread; i++) {
      char c = buf[i];
      if (c == '\n') nl++;
      if (inword && !isalpha(c)) inword = false;
      else if (!inword && isalpha(c)) { inword = true; nw++; }
    }
  }
  if (filename) printf("% 8d% 8d% 8d %s\n", nl, nw, nb, filename);
  else printf("% 8d% 8d% 8d\n", nl, nw, nb);
  *nlines += nl; *nwords += nw; *nbytes += nb;
}

void func_wc(ShellInfo* shell) {
  int nlines = 0, nwords = 0, nbytes = 0;
  char path[1024]; int iter = strlen("wc");
  for (;;) if (shell->current_command.argument[iter] == ' ') iter++; else break;
  strcpy(path, shell->current_command.argument + iter);
  if (path[0] == '\0') {
    wc(stdin, NULL, &nlines, &nwords, &nbytes);
    return;
  }
  char* args[128];
  int count = split_blank(args, path);
  for (int i = 0; i < count; i++) {
    char buf[1024]; strcpy(buf, args[i]);
    if (!file_exist(shell, buf)) continue;
    FILE* fp = fopen(buf, "r");
    if (!fp) { perror("fopen"); continue; }
    wc(fp, buf, &nlines, &nwords, &nbytes);
    fclose(fp);
  }
  if (count > 1) printf("% 8d% 8d% 8d total\n", nlines, nwords, nbytes);
}

void func_pwd(ShellInfo* shell) {
  if (!check_arg(shell->current_command.argument, "pwd")) { printf("Error : There is a character after \"pwd\"\n"); return; }
  printf("%s > %s\n", shell->user_name, shell->current_dir);
}

int split_blank(char** args, char* arg) {
  int count = 0;
  for (;;) {
    while (*arg == ' ') arg++;
    if (*arg == '\0') break;
    args[count++] = arg;
    while (*arg && *arg != ' ') arg++;
    if (*arg == '\0') break;
    *arg++ = '\0';
  }
  args[count] = NULL;
  return count;
}

void execute_command(ShellInfo* shell) {
  int pid, status;
  if ((pid = fork()) == -1) { exit(1); }
  if (pid == 0) {
    char* args[128];
    int count = split_blank(args, shell->current_command.argument);
    execvp(args[0], args);
    perror(args[0]);
    exit(-1);
  }
  if (shell->current_command.status == background) return;
  wait(&status);
}

bool does_exist_directory(char* path) {
  path_reflesh(path);
  DIR* d;
  if ((d = opendir(path)) == NULL) { perror(path); return false; }
  closedir(d);
  return true;
}

void path_reflesh(char* dir) {
  int now_iter = 0, read_iter = 0;
  char path[1024];
  for (; read_iter < strlen(dir); now_iter++) {
    for (;;) {
      bool is_ok = true;
      if (starts_with(dir + read_iter, " ")) { read_iter += 1; is_ok = false; }
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
