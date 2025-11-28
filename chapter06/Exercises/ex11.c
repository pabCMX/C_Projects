// Calculate the value of e using infinite series to the nth term.
#include <stdio.h>

int main(void) {
  int    finalTerm;
  double   denomFactorial = 1.0;
  double sum = 0.0;

  printf("This program computes e to the nth term of the fractional infinite series.\n");
  printf("Enter the limit term for e: ");
  scanf("%d", &finalTerm);

  for (int i = 0; i <= finalTerm; i++) {
    if (i != 0) {
      denomFactorial *= i;
    }
    sum += 1.0 / denomFactorial;
  }

  printf("The number e to %d term(s) is %.16f\n", finalTerm, sum);

  return 0;
}
