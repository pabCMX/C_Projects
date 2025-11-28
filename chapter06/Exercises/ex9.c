// Extending Ch. 2 ex8.c to display the balance after user entered number of payments.
// I took the liberty of adding a proper payment calculation with pow() as well.
// Remember to build with the -lm flag to link the math library.
#include <stdio.h>
#include <math.h>

int main(void) {
  double principal, interest, periodInterest, expInterest, balance, payment;
  int    periods;
  printf("Enter loan amount: $");
  scanf("%lf", &principal);
  printf("Enter interest rate (%%): ");
  scanf("%lf", &interest);
  interest /= 100.0; // Convert percent to decimal.
  periodInterest = interest / 12.0;
  printf("Enter number of monthly payments: ");
  scanf("%d", &periods);

  // Now we calculate the payment amount:
  expInterest = pow(1 + periodInterest, periods);
  payment            = (principal * ((periodInterest * expInterest) / (expInterest - 1)));
  // Avoiding changing input.
  balance = principal;
  for (int i = 1; i <= periods; i++) {
    balance = (balance * (1 + periodInterest)) - payment;
    printf("Payment: $%.2lf, Balance after payment %2d: $%10.2f\n", payment, i, balance);
  }

  return 0;
}
