#include <stdio.h>

//Print Fahrenheit-Celsius table
// for fahr = 0, 20, ..., 300

int main(void)
{
  double fahr, celsius, factor;
  double lower, upper, step;

  lower = 0.0;
  upper = 300.0;
  step = 20.0;
  factor = (5.0/9.0);

  fahr = lower;
  printf("fahr\tcelsius\n");
  while (fahr <= upper) {
    celsius = factor * (fahr - 32.0 );
    printf("%3.0f\t%6.2f\n", fahr, celsius);
    fahr = fahr + step;
  }
  return 0;
}