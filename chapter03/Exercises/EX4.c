// Transform user phone number to dotted format.
#include <stdio.h>

int main(void) {
  int areaCode, firstTriplet, secondQuartet;

  printf("Enter phone number [(xxx) xxx-xxxx]: ");
  scanf("(%d) %d-%d", &areaCode, &firstTriplet, &secondQuartet);

  printf("You entered: %03d.%03d.%04d\n", areaCode, firstTriplet, secondQuartet);

  return 0;
}
