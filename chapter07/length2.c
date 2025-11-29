// Determine length of message, now with C idiom character loop.
#include <stdio.h>

int main(void) {
  int len = 0;

  printf("Enter a message: ");
  while (getchar() != '\n') {
    len++;
  }

  printf("Your message was %d characters long.\n", len);

  return 0;
}
