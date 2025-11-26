// Calculate the earlier date from two user dates.
#include <stdio.h>

int main(void) {
  int year1, year2, month1, month2, day1, day2;

  printf("Enter your first date (mm/dd/yyyy): ");
  scanf("%2d / %2d / %4d", &month1, &day1, &year1);

  printf("Enter your second date (mm/dd/yyyy): ");
  scanf("%2d / %2d / %4d", &month2, &day2, &year2);

  if (year1 > year2) {
    printf("%.2d/%.2d/%.4d is earlier than %.2d/%.2d/%.4d\n", month2, day2, year2, month1, day1,
           year1);
  } else if (year1 != year2) {
    printf("%.2d/%.2d/%.4d is earlier than %.2d/%.2d/%.4d\n", month1, day1, year1, month2, day2,
           year2);
  } else if (month1 > month2) {
    printf("%.2d/%.2d/%.4d is earlier than %.2d/%.2d/%.4d\n", month2, day2, year2, month1, day1,
           year1);
  } else if (month1 != month2) {
    printf("%.2d/%.2d/%.4d is earlier than %.2d/%.2d/%.4d\n", month1, day1, year1, month2, day2,
           year2);
  } else if (day1 > day2) {
    printf("%.2d/%.2d/%.4d is earlier than %.2d/%.2d/%.4d\n", month2, day2, year2, month1, day1,
           year1);
  } else if (day1 != day2) {
    printf("%.2d/%.2d/%.4d is earlier than %.2d/%.2d/%.4d\n", month1, day1, year1, month2, day2,
           year2);
  } else {
    printf("%.2d/%.2d/%.4d and %.2d/%.2d/%.4d are the same day.\n", month2, day2, year2, month1,
           day1, year1);
  }

  return 0;
}
