#include <stdio.h>

int main(void) {
  const float PI = 3.14159265359;

  float volume, radius, radiusCubed;

  printf("Enter radius(meters): ");
  scanf("%f", &radius);

  radiusCubed = radius * radius * radius;
  volume      = (4.0 / 3.0) * PI * radiusCubed;

  printf("Radius(meters): %.2f, Volume(meters cubed): %.2f", radius, volume);

  return 0;
}
