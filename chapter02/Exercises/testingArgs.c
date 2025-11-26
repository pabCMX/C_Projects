// C program to illustrate
// command line arguments
#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("Program name is: %s\n", argv[0]);

  if (argc == 1)
    printf("\nNo Arguments Passed\n");

  if (argc >= 2) {
    printf("\nNumber Of Arguments Passed: %d\n", argc);
    for (int i = 0; i < argc; i++)
      printf("\nargv[%d]: %s\n", i, argv[i]);
  }
  return 0;
}
