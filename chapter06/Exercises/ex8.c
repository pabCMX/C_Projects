// Program that prints a one month calendar from user starting day and month days.
#include <stdio.h>

int main(void) {
  int startDay, totalDays;

  printf("Enter number of days in month: ");
  scanf("%d", &totalDays);
  printf("What day does the month start on (1=Sun, 7=Sat): ");
  scanf("%d", &startDay);

  for (int i = 1; i < totalDays + startDay; i++) {
    if (startDay > i) {
      printf("\t");
      continue;
    }
    printf("%d\t", i - startDay + 1);
    if (i % 7 == 0) {
      printf("\n");
    }
  }
  // Final newline for certainty.
  printf("\n");

  return 0;
}
