#include <stdio.h>

int main (void)
{
  float x, total;

  printf("Enter value for x: ");
  scanf("%f", &x);

  total = ((((3*x + 2)*x - 5)*x - 1)*x + 7)*x - 6;
  printf("Value is: %f", total);

  return 0;
}