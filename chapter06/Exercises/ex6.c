// Return all even squares between 1 and user input.
#include <stdio.h>

int main(void) {
  int input;

  printf("Input end-bound for squares: ");
  scanf("%d", &input);

  for (int i = 1; i * i <= input; i++) {
    // Since even squares must have even roots, we only need to check i
    if (i % 2 == 0) {
      // Print the root and the square.
      printf("%10d%10d\n", i, i * i);
    }
  }

  return 0;
}
