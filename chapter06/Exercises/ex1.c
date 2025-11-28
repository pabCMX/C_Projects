// Return the largest number among a series of user input numbers.
#include <stdio.h>

int main(void) {
  double input = 1, max;

  printf("Enter a positive number (0 to exit): ");
  scanf("%lf", &input);
  max = input;
  while (input > 0) {
    if (input > max) {
      max = input;
    }
    printf("Enter a positive number (0 to exit): ");
    scanf("%lf", &input);
  }
  printf("The largest number entered was: %lf\n", max);

  return 0;
}
