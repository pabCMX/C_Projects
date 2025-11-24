// Enter a two digit number and return the number reversed.
#include <stdio.h>

int main(void) {
  int input, tensDigit, onesDigit;

  printf("Enter a two-digit number: ");
  scanf("%d", &input);

  onesDigit = input % 10;
  tensDigit = input / 10;

  printf("The reversal is: %d%d", onesDigit, tensDigit);

  return 0;
}
