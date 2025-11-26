// Converts 24 hour time to 12 hour time
// Pitfalls include not managing am/pm for 12 pm and 12 am, as well as missing digits on minutes.
#include <stdio.h>

int main(void) {
  int userHours, userMinutes, convertedHours;

  printf("Enter a 24-hour time (HH:MM): ");
  scanf("%2d:%2d", &userHours, &userMinutes);

  if (userHours >= 13) {
    convertedHours = userHours - 12;
    printf("Equivalent 12-hour time: %.2d:%.2d PM\n", convertedHours, userMinutes);
  } else if (userHours == 12) {
    convertedHours = userHours;
    printf("Equivalent 12-hour time: %.2d:%.2d PM\n", convertedHours, userMinutes);
  } else if (userHours > 0) {
    convertedHours = userHours;
    printf("Equivalent 12-hour time: %.2d:%.2d AM\n", convertedHours, userMinutes);
  } else {
    convertedHours = 12;
    printf("Equivalent 12-hour time: %.2d:%.2d AM\n", convertedHours, userMinutes);
  }

  return 0;
}
