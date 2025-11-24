// Calculates a resident's income tax given a total taxable income.
#include <stdio.h>

int main(void) {
  float income, tax;

  printf("Enter your yearly taxable income: ");
  scanf("%f", &income);

  if (income < 750) {
    tax = income * 0.01f;

  } else if (income < 2250) {
    tax = ((income - 750) * 0.02f) + 7.5f;

  } else if (income < 3750) {
    tax = ((income - 2250) * 0.03f) + 7.5f + 30.0f;

  } else if (income < 5250) {
    tax = ((income - 3750) * 0.04f) + 7.5f + 30.0f + 45.0f;

  } else if (income < 7000) {
    tax = ((income - 5250) * 0.05f) + 7.5f + 30.0f + 45.0f + 60.0f;
  } else {
    tax = ((income - 7000) * 0.06f) + 7.5f + 30.0f + 45.0f + 60.0f + 87.5f;
  }

  printf("The tax due on your income of $%5.2f is: $%5.2f", income, tax);

  return 0;
}
