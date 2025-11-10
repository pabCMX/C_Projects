#include <stdio.h>
#include <math.h>

#define _USE_MATH_DEFINES

int main (int rad) 
{
  float volume, radius, radiusCubed;

  printf("Enter radius(meters): ");
  scanf("%f", &radius);

  radiusCubed = radius * radius * radius;
  volume = (4.0/3.0)*M_PI*radiusCubed;
  
  printf("Radius(meters): %.2f, Volume(meters cubed): %.2f", radius, volume);

  return 0;
}