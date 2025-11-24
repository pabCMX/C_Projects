// Ex2 without using math, instead input splits.
#include <stdio.h>

int main(void) {
  int hundreds, tensDigit, onesDigit;

  printf("Enter a three-digit number: ");
  scanf("%1d%1d%1d", &hundreds, &tensDigit, &onesDigit);

  printf("The reversal is: %d%d%d", onesDigit, tensDigit, hundreds);

  return 0;
}
