// Calculates digits in a given number. We can assume the number is no bigger than 4 digits.
#include <stdio.h>

int main(void) {
  int userNum;
  int count;

  printf("Enter a number: ");
  scanf("%d", &userNum);

  // To make sure negatives don't break it, we'll throw the sign away
  if (userNum < 0) {
    userNum = -userNum;
  }

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

  printf("The number %d has %d digits.\n", userNum, count);

  return 0;
}
