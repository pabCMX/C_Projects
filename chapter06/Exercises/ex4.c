// Update broker.c to allow for continuous inputs
#include <stdio.h>

int main(void) {
  float commission, value;

  for (;;) {
    printf("Enter value of trade (exit with 0): ");
    scanf("%f", &value);
    if (value <= 0) {
      break;
    }

    if (value < 2'500.00f)
      commission = 30.00f + .017f * value;
    else if (value < 6'250.00f)
      commission = 56.00f + .0066f * value;
    else if (value < 20'000.00f)
      commission = 76.00f + .0034f * value;
    else if (value < 50'000.00f)
      commission = 100.00f + .0022f * value;
    else if (value < 500'000.00f)
      commission = 155.00f + .0011f * value;
    else
      commission = 255.00f + .0009f * value;

    if (commission < 39.00f) {
      commission = 39.00f;
    }

    printf("Commission: $%.2f\n", commission);
  }
  return 0;
}
