// Return english number name for given two digit number.
#include <stdio.h>

int main(void) {
  int tensDigit, onesDigit;

  printf("Enter a two-digit number: ");
  scanf("%1d%1d", &tensDigit, &onesDigit);

  printf("You entered the number ");

  if (tensDigit == 1) {
    switch (onesDigit) {
    case 9:
      printf("nineteen\n");
      break;
    case 8:
      printf("eighteen\n");
      break;
    case 7:
      printf("seventeen\n");
      break;
    case 6:
      printf("sixteen\n");
      break;
    case 5:
      printf("fifteen\n");
      break;
    case 4:
      printf("fourteen\n");
      break;
    case 3:
      printf("thirteen\n");
      break;
    case 2:
      printf("twelve\n");
      break;
    case 1:
      printf("eleven\n");
      break;
    case 0:
      printf("ten\n");
      break;
    default:
      printf("Error: Invalid number.\n");
      return 1;
    }
    return 0;
  }

  switch (tensDigit) {
  case 9:
    printf("ninety");
    break;
  case 8:
    printf("eighty");
    break;
  case 7:
    printf("seventy");
    break;
  case 6:
    printf("sixty");
    break;
  case 5:
    printf("fifty");
    break;
  case 4:
    printf("forty");
    break;
  case 3:
    printf("thirty");
    break;
  case 2:
    printf("twenty");
    break;
  default:
    printf("Error: Invalid number\n");
    return 1;
  }

  switch (onesDigit) {
  case 9:
    printf("-nine\n");
    break;
  case 8:
    printf("-eight\n");
    break;
  case 7:
    printf("-seven\n");
    break;
  case 6:
    printf("-six\n");
    break;
  case 5:
    printf("-five\n");
    break;
  case 4:
    printf("-four\n");
    break;
  case 3:
    printf("-three\n");
    break;
  case 2:
    printf("-two\n");
    break;
  case 1:
    printf("-one\n");
    break;
  case 0:
    break;
  default:
    printf("Error: Invalid number\n");
    return 1;
  }

  return 0;
}
