// Prints a table of squares using 'for'
#include <stdio.h>

int main(void) {
  int i, n;

  printf("This prgram prints a table of squares.\n");
  printf("Enter the number of entries in table: ");
  scanf("%d", &n);

  for (i = 1; i <= n; i++) {
    printf("%10d%10d\n", i, i * i);
  }

  return 0;
}
