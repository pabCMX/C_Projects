#include <stdio.h>

int main(void)
{
  int areaCode, firstTriplet, secondQuartet;

  printf("Enter phone number [(xxx) xxx-xxxx]: ");
  scanf("(%d) %d-%d", &areaCode, &firstTriplet, &secondQuartet);

  printf("You entered: %d.%d.%d", areaCode, firstTriplet, secondQuartet);

  return 0;
}