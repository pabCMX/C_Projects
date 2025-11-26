// Return the largest number among a series of user input numbers.
#include <stdio.h>

int main(void) {
  double input = 1, max = 0;

  while (input > 0) {
    printf("Enter a number: ");
    scanf("%lf", &input);
    if (input > max) {
      max = input;
    }
  }
  printf("The largest number entered was: %lf", max);

  return 0;
}
