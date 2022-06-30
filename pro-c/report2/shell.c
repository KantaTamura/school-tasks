#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef enum CommandStatus {
  foreground,
  background,
  endshell,
  none,
} CommandStatus;

typedef struct Command {
  char command_name[1024];
  void (*exe_command)();
  char argument[1024];
  CommandStatus status;
} Command;

typedef struct ShellInfo {
  char user_name[256];
  char current_dir[1024];
  Command buffer_command[32];
  uint8_t buffer_command_num;
} ShellInfo;

Command new_command(char* name, void (*exe_command)(ShellInfo*, char*), CommandStatus status, char* argument);
Command new_command_exit();
Command new_command_none();
ShellInfo new_shell_info();

bool starts_with(char* p, char* q);

Command parse_command(char* str, size_t length);
void func_cd(ShellInfo*, char*);

int main() {
  // TODO: Buffer the shell and display the original shell on return
  system("clear");

  ShellInfo main_shell = new_shell_info();

  for (;;) {
    char input_string_buffer[1024] = "";
    size_t input_string_length = 0;

    printf("%s > %s\n", main_shell.user_name, main_shell.current_dir);
    printf("Command: ");
    if (scanf("%1023[^\n]%zn%*[^\n]", input_string_buffer, &input_string_length) == EOF) {
      fprintf(stderr, "Can't read command\n");
      exit(-1);
    }
    scanf("%*c");

    printf("\x1b[33minput command : %s, length : %zu\n\x1b[39m", input_string_buffer, input_string_length);

    Command current_command = parse_command(input_string_buffer, input_string_length);

    switch (current_command.status) {
    case foreground:
    case background:
      current_command.exe_command(&main_shell, current_command.argument);
      continue;
    case endshell:
      printf("exit shell\n");
      exit(0);
    case none:
      continue;
    }
  }
}

Command parse_command(char* str, size_t length) {
  for (;;str += 1) if (*str != ' ') break;
  if (starts_with(str, "exit"))
    return new_command_exit();
  if (starts_with(str, "cd"))
    return new_command("cd", func_cd, foreground, str);
  return new_command_none();
}

// TODO: error handling
Command new_command(char* name, void (*exe_command)(ShellInfo*, char*), CommandStatus status, char* argument) {
  Command command;
  strcpy(command.command_name, name);
  command.exe_command = exe_command;
  command.status = status;
  strcpy(command.argument, argument + strlen(name) + 1);
  return command;
}

Command new_command_exit() {
  Command command;
  command.status = endshell;
  return command;
}

Command new_command_none() {
  Command command;
  command.status = none;
  return command;
}

// TODO: error handling
ShellInfo new_shell_info() {
  ShellInfo shell;
  strcpy(shell.user_name, getenv("USER"));
  strcpy(shell.current_dir, getenv("HOME"));
  shell.buffer_command_num = 0;
  return shell;
}

bool starts_with(char* p, char* q) {
  return strncmp(p, q, strlen(q)) == 0;
}

void func_cd(ShellInfo* shell, char* dir) {
  struct stat statBuf;
  if (stat(dir, &statBuf) != 0) {
    printf("\"%s\": No such file or directory", dir);
    return;
  }
  
}
