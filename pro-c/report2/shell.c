#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct Command {
  char command_name[1024];
  size_t command_length;
} Command;

typedef struct ShellInfo {
  char current_dir[1024];
  Command buffer_command[5];
  uint8_t buffer_command_num;
} ShellInfo;

int main() {
  system("clear");
  ShellInfo main_shell;
  //TODO: error handling
  strcpy(main_shell.current_dir, "~");
  main_shell.buffer_command_num = 0;
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
