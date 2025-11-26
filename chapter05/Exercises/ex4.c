// Computes Beaufort scale from given wind speed in knots
#include <stdio.h>

int main(void) {
  float windSpeed;

  printf("Enter your wind speed (in knots): ");
  scanf("%f", &windSpeed);

  if (windSpeed < 1) {
    printf("Calm.\n");
  } else if (windSpeed <= 3) {
    printf("Very light air.\n");
  } else if (windSpeed <= 6) {
    printf("Light breeze.\n");
  } else if (windSpeed <= 10) {
    printf("Gentle breeze.\n");
  } else if (windSpeed <= 16) {
    printf("Moderate breeze.\n");
  } else if (windSpeed <= 21) {
    printf("Fresh or cooling breeze.\n");
  } else if (windSpeed <= 27) {
    printf("Strong breeze.\n");
  } else if (windSpeed <= 33) {
    printf("Near Gale, High wind.\n");
  } else if (windSpeed <= 40) {
    printf("Gale Winds.\n");
  } else if (windSpeed <= 47) {
    printf("Severe Gale.\n");
  } else if (windSpeed <= 55) {
    printf("Storm, Whole Gale.\n");
  } else if (windSpeed <= 63) {
    printf("Violent Storm\n");
  } else {
    printf("Hurricane force winds.\n");
  }

  return 0;
}
