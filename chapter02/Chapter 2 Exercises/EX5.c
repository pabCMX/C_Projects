#include <stdio.h>

int main (void)
{
  float x, xFifthDegree, xFourthDegree, xThirdDegree, xSecondDegree, total;

  printf("Enter value for x: ");
  scanf("%f", &x);

  xFifthDegree = x * x * x * x * x;
  xFourthDegree = x * x * x * x;
  xThirdDegree = x * x * x;
  xSecondDegree = x * x;

  total = (3*xFifthDegree) + (2*xFourthDegree) - (5*xThirdDegree) - xSecondDegree + (7*x) - 6;
  printf("Value is: %f", total);

  return 0;
}