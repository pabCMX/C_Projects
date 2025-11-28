// Generalize chapter04 ex1.c to any number of digits.
#include <stdio.h>

int main(void) {
  int input;

  printf("Enter a positive integer: ");
  scanf("%d", &input);

  printf("The reversal is: ");
  do {
    printf("%d", input % 10);
    input /= 10;
  } while (input > 0);

  printf("\n");
  return 0;
}
