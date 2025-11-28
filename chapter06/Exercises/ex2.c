// Calculate the greatest common divisor (GCD) from two input integers
#include <stdio.h>

int main(void) {
  int m, n, GCD, temp;

  printf("Enter two integers, separated by a space: ");
  scanf("%d %d", &m, &n);

  // We'll use euclid's algorithm, first testing if n is zero.
  while (n > 0) {
    // then m mod n
    temp = m % n;
    m    = n;
    n    = temp;
  }
  GCD = m;

  printf("Greatest common divisor: %d\n", GCD);

  return 0;
}
