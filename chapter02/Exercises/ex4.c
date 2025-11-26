// Compute total with tax from user input subtotal
#include <stdio.h>

int main(void) {
  float subtotal, total, tax, taxPercent = 0.05f;

  printf("Enter an amount: $");
  scanf("%f", &subtotal);

  tax   = subtotal * taxPercent;
  total = subtotal + tax;

  printf("With tax added: $%.2f\n", total);

  return 0;
}
