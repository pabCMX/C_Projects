// Calculates a resident's income tax given a total taxable income.
#include <stdio.h>

int main(void) {
  float income, tax;

  printf("Enter your yearly taxable income: ");
  scanf("%f", &income);

  if (income < 750) {
    tax = income * 0.01f;

  } else if (income < 2'250) {
    tax = ((income - 750) * 0.02f) + 7.5f;

  } else if (income < 3'750) {
    tax = ((income - 2'250) * 0.03f) + 7.5f + 30.0f;

  } else if (income < 5'250) {
    tax = ((income - 3'750) * 0.04f) + 7.5f + 30.0f + 45.0f;

  } else if (income < 7'000) {
    tax = ((income - 5'250) * 0.05f) + 7.5f + 30.0f + 45.0f + 60.0f;
  } else {
    tax = ((income - 7'000) * 0.06f) + 7.5f + 30.0f + 45.0f + 60.0f + 87.5f;
  }

  printf("The tax due on your income of $%5.2f is: $%5.2f\n", income, tax);

  return 0;
}
