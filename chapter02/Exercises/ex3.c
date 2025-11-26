// Refactor ex2.c to take user input for radius
#include <stdio.h>

int main(void) {
  const float PI = 3.14159265359;

  float volume, radius, radiusCubed;

  printf("Enter radius(meters): ");
  scanf("%f", &radius);

  radiusCubed = radius * radius * radius;
  volume      = (4.0f / 3.0f) * PI * radiusCubed;

  printf("Radius(meters): %.2f, Volume(meters cubed): %.2f\n", radius, volume);

  return 0;
}
