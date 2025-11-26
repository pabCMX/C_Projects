// Compute the volume of a sphere of 10 meter radius.
#include <stdio.h>

int main(void) {
  const float PI = 3.14159265359;

  float volume;
  float radius = 10.0f;
  float radiusCubed;

  radiusCubed = radius * radius * radius;

  volume = (4.0f / 3.0f) * PI * radiusCubed;

  printf("Radius: %.1f, Volume: %.2f\n", radius, volume);

  return 0;
}
