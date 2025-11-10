#include <stdio.h>
#include <math.h>

#define _USE_MATH_DEFINES

int main (void) 
{
  float volume, radius = 10.0f, radiusCubed;

  radiusCubed = radius * radius * radius;

  volume = (4.0f/3.0f)*M_PI*radiusCubed;
  
  printf("Radius: %.1f, Volume: %.2f", radius, volume);

  return 0;
}