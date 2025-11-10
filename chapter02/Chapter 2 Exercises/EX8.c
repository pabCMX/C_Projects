#include <stdio.h>

int main (void)
{
  float principal, interest, payment, balance;
  printf("Enter loan amount: $");
  scanf("%f", &principal);
  printf("Enter interest rate (%%): ");
  scanf("%f", &interest);
  interest = interest / 100;
  printf("Enter payment: $");
  scanf("%f", &payment);

  balance = (principal * (1+(interest / 12.0))) - payment;
  printf("Balance after first payment: $%.2f\n", balance);

  principal = balance;
  balance = (principal * (1+(interest / 12.0))) - payment;
  printf("Balance after second payment: $%.2f\n", balance);
  
  principal = balance;
  balance = (principal * (1+(interest / 12.0))) - payment;
  printf("Balance after third payment: $%.2f\n", balance);

  return 0;
}