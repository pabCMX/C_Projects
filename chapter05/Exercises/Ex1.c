// Calculates digits in a given number.
#include <stdio.h>

int main(void) {
  int userNum;
  int count;

  printf("Enter a number: ");
  scanf("%d", &userNum);

  if (userNum <= 9) {
    count = 1;
  } else if (userNum <= 99) {
    count = 2;
  } else if (userNum <= 999) {
    count = 3;
  } else if (userNum <= 9999) {
    count = 4;
  } else {
    count = 5;
  }

  printf("The number %d has %d digits.", userNum, count);

  return 0;
}
