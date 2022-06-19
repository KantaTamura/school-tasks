#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  for (;;) {
    char input_string_buffer[1024] = "";
    size_t input_string_length = 0;

    printf("> ");
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
