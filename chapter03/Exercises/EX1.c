// Transform user input mm/dd/yyyy to yyyy/mm/dd
#include <stdio.h>

int main(void) {
  int year, month, day;

  printf("Enter a date (mm/dd/yyyy): ");
  scanf("%d / %d / %d", &month, &day, &year);

  printf("You entered the date: %d/%d/%d\n", year, month, day);

  return 0;
}
