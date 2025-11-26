// Program that find max and min of 4 given integers
#include <stdio.h>

int main(void) {
  int i1, i2, i3, i4, swap;
  int max = 0;
  int min = 0;

  printf("Enter four integers separated by spaces: ");
  scanf("%d %d %d %d", &i1, &i2, &i3, &i4);

  if (i2 > i1) {
    swap = i1;
    i1   = i2;
    i2   = swap;
  }

  if (i4 > i3) {
    swap = i3;
    i3   = i4;
    i4   = swap;
  }
  // Check the smallest for min
  if (i2 > i4) {
    min = i4;
  } else {
    min = i2;
  }

  // Check the largest for max
  if (i1 > i3) {
    max = i1;
  } else {
    max = i3;
  }

  printf("Largest: %d\nSmallest: %d\n", max, min);

  return 0;
}
