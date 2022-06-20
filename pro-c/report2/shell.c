#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef enum CommandStatus {
  foreground,
  background,
  endshell,
  none,
} CommandStatus;

typedef struct Command {
  char command_name[1024];
  void (*exe_command)();
  CommandStatus status;
} Command;

typedef struct ShellInfo {
  char user_name[256];
  char current_dir[1024];
  Command buffer_command[32];
  uint8_t buffer_command_num;
} ShellInfo;

Command new_command(char* name, CommandStatus status);
ShellInfo new_shell_info();

bool starts_with(char* p, char* q);

Command parse_command(char* str, size_t length);

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

    printf("input command : %s, length : %zu\n", input_string_buffer, input_string_length);

    Command current_command = parse_command(input_string_buffer, input_string_length);

    switch (current_command.status) {
    case foreground:
    case background:
      continue;
    case endshell:
      printf("exit shell\n");
      exit(0);
    case none:
      continue;
    }
  }
}

// TODO: error handling
Command new_command(char* name, CommandStatus status) {
  Command command;
  strcpy(command.command_name, name);
  command.status = status;
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

Command parse_command(char* str, size_t length) {
  if (starts_with(str, "exit")) {
    
    return new_command("exit", endshell);
  }
  return new_command("none", none);
}
