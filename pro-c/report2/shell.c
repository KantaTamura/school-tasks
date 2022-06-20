#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef struct Command {
  char command_name[1024];
  size_t command_length;
} Command;

typedef struct ShellInfo {
  char current_dir[1024];
  Command buffer_command[5];
  uint8_t buffer_command_num;
} ShellInfo;

Command new_command();
ShellInfo new_shell_info();

int main() {
  system("clear");
  
  ShellInfo main_shell = new_shell_info();
  
  for (;;) {
    char input_string_buffer[1024] = "";
    size_t input_string_length = 0;

    printf("%s > ", main_shell.current_dir);
    if (scanf("%1023[^\n]%zn%*[^\n]", input_string_buffer, &input_string_length) == EOF) {
      fprintf(stderr, "Can't read command\n");
      exit(-1);
    }
    scanf("%*c");

    printf("input command : %s, length : %zu\n", input_string_buffer, input_string_length);
    
    if (strcmp(input_string_buffer, "exit") == 0) {
      printf("exit shell\n");
      exit(0);
    }
  }
}

// TODO: error handling
Command new_command() {
  Command command;
  strcpy(command.command_name, "");
  command.command_length = 0;
  return command;
}

// TODO: error handling
ShellInfo new_shell_info() {
  ShellInfo shell;
  strcpy(shell.current_dir, getenv("HOME"));
  shell.buffer_command_num = 0;
  return shell;
}
