#include <stdio.h>

int main (void)
{
  int i;
  float x;

  i = 40;
  x = 839.21f;

  //prints a set of different justifications and floating point precisions.
  printf("|%d|%5d|%-5d|%5.3d|\n", i, i, i, i);
  printf("|%10.3f|%10.3e|%-10g|\n", x, x, x);

  //prints a set of escape sequences
  printf("\"Hello!\"");
  printf("Item\tUnit\tPurchase\n\tPrice\tDate");

  return 0;
}