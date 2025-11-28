// Extend Ch. 5 ex9.c to take any number of dates, and find the earliest one.
#include <stdio.h>

int main(void) {
  int year1, year2, month1, month2, day1, day2;

  printf("Enter a date (mm/dd/yyyy) [0/0/0 to exit]: ");
  scanf("%d / %d / %d", &month1, &day1, &year1);

  for (;;) {
    printf("Enter a date (mm/dd/yyyy) [0/0/0 to exit]: ");
    scanf("%d / %d / %d", &month2, &day2, &year2);

    if (month2 == 0 && day2 == 0 && year2 == 0) {
      break;
    }

    if (year1 > year2) {
      month1 = month2;
      day1   = day2;
      year1  = year2;
    } else if (year1 < year2) {
      continue;
    } else if (month1 > month2) {
      month1 = month2;
      day1   = day2;
      year1  = year2;
    } else if (month1 < month2) {
      continue;
    } else if (day1 > day2) {
      month1 = month2;
      day1   = day2;
      year1  = year2;
    } else if (day1 < day2) {
      continue;
    }
  }

  printf("%02d/%02d/%04d is the earliest date.\n", month1, day1, year1);
  return 0;
}
