// Update to upc.c to state whether UPC given is valid.
#include <stdio.h>

int main(void) {
  int d, i1, i2, i3, i4, i5, j1, j2, j3, j4, j5, checksum, first_sum, second_sum, total,
      computedCheck;

  printf("Enter a complete UPC: ");
  scanf("%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d", &d, &i1, &i2, &i3, &i4, &i5, &j1, &j2, &j3, &j4,
        &j5, &checksum);

  first_sum     = d + i2 + i4 + j1 + j3 + j5;
  second_sum    = i1 + i3 + i5 + j2 + j4;
  total         = 3 * first_sum + second_sum;
  computedCheck = 9 - ((total - 1) % 10);

  if (computedCheck == checksum) {
    printf("The UPC is valid.\n");
  } else {
    printf("The UPC is invalid. Checksum should be %d, got %d\n", computedCheck, checksum);
  }

  return 0;
}
