// Rearrange square3.c to initialize variables within the 'for' loop.
#include <stdio.h>

int main(void) {
  int n;

  printf("This program prints a table of squares.\n");
  printf("Enter the number of entries in table: ");
  scanf("%d", &n);

  for (int i = 1, square = 1, odd = 3; i <= n; odd += 2, i++) {
    printf("%10d%10d\n", i, square);
    square += odd;
  }

  return 0;
}
