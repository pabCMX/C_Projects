// Reduce user fraction to lowest terms.
#include <stdio.h>

int main(void) {
  int numer, denom, GCD, temp, m, n;

  printf("Enter a fraction (n/m): ");
  scanf("%d / %d", &numer, &denom);
  // Don't pollute the original inputs
  n = numer;
  m = denom;
  // Use Euclid's GCD algo
  while (n > 0) {
    temp = m % n;
    m    = n;
    n    = temp;
  }

  // Apply the GCD
  GCD = m;
  numer /= GCD;
  denom /= GCD;

  printf("In lowest terms: %d/%d\n", numer, denom);

  return 0;
}
