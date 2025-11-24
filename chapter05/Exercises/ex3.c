// Update to broker.c to calculate from shares and share price
// and compare to rival broker commission fees.
#include <stdio.h>

int main(void) {
  int   shareCount;
  float commission, rivalCommission, sharePrice, tradeValue;

  printf("Enter share price: ");
  scanf("%f", &sharePrice);
  printf("Enter number of shares: ");
  scanf("%d", &shareCount);
  tradeValue = sharePrice * shareCount;

  if (tradeValue < 2'500.00f)
    commission = 30.00f + .017f * tradeValue;
  else if (tradeValue < 6'250.00f)
    commission = 56.00f + .0066f * tradeValue;
  else if (tradeValue < 20'000.00f)
    commission = 76.00f + .0034f * tradeValue;
  else if (tradeValue < 50'000.00f)
    commission = 100.00f + .0022f * tradeValue;
  else if (tradeValue < 500'000.00f)
    commission = 155.00f + .0011f * tradeValue;
  else
    commission = 255.00f + .0009f * tradeValue;

  if (commission < 39.00f)
    commission = 39.00f;

  if (shareCount < 2000) {
    rivalCommission = 33.00f + 0.03f * shareCount;
  } else {
    rivalCommission = 33.00f + 0.02f * shareCount;
  }
  printf("Our Commission: $%.2f\nTheir Commission: $%.2f", commission, rivalCommission);

  return 0;
}
