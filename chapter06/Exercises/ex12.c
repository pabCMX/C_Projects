// Modify ex11.c to calculate until certain decimal place accuracy.
#include <stdio.h>

int main(void) {
  double   denomFactorial = 1.0;
  double sum = 0.0, accuracy;

  printf("This program computes e with a fractional infinite series up to a certain term contribution threshold.\n");
  printf("Enter the term contribution threshold for e (e.g. 0.000001): ");
  scanf("%lf", &accuracy);

  for (int i = 0;; i++) {
    if (i != 0) {
      denomFactorial *= i;
    }
    if (1.0 / denomFactorial <= accuracy) {
      break;
    }
    sum += 1.0 / denomFactorial;
  }

  printf("The number e to %f term contribution threshold is %.16f\n", accuracy, sum);
  
  return 0;
}
