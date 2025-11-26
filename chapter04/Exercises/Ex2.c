// Extending Ex1 to handle 3 digit numbers
#include <stdio.h>

int main(void) {
  int input, hundreds, tensDigit, onesDigit;

  printf("Input a 3 digit number: ");
  scanf("%d", &input);

  hundreds  = input / 100;
  onesDigit = input % 10;
  tensDigit = (input % 100) / 10;

  printf("The reversal is: %d%d%d\n", onesDigit, tensDigit, hundreds);

  return 0;
}
