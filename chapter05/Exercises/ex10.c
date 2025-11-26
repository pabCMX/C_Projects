// Convert numerical grade into letter grade with "switch"
#include <stdio.h>

int main(void) {
  int input, tensDigit;

  printf("Enter your numerical grade: ");
  scanf("%d", &input);

  if (input < 0) {
    printf("Error: numerical grade greater than 100 or less than 0.\n");
    return 1;
  }

  tensDigit = input / 10;

  switch (tensDigit) {
  case 10:
  case 9:
    printf("Letter grade: A\n");
    break;
  case 8:
    printf("Letter grade: B\n");
    break;
  case 7:
    printf("Letter grade: C\n");
    break;
  case 6:
    printf("Letter grade: D\n");
    break;
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
  case 0:
    printf("Letter grade: F\n");
    break;
  default:
    printf("Error: numerical grade greater than 100 or less than 0.\n");
    return 1;
  }

  return 0;
}
