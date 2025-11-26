// Find the closest departure and arrival pair given a desired time.
#include <stdio.h>

int main(void) {
  int inputHour, inputMinutes, minutesSinceMidnight;
  int d1, d2, d3, d4, d5, d6, d7, d8;

  // Cleanest way to calculate departure midpoints is to have departure variables.
  d1 = 8 * 60;       // 8:00 AM
  d2 = 9 * 60 + 43;  // 9:43 AM
  d3 = 11 * 60 + 19; // 11:19 AM
  d4 = 12 * 60 + 47; // 12:47 PM
  d5 = 14 * 60;      // 2:00 PM
  d6 = 15 * 60 + 45; // 3:45 PM
  d7 = 19 * 60;      // 7:00 PM
  d8 = 21 * 60 + 45; // 9:45 PM

  printf("Enter a 24-hour time: ");
  scanf("%2d:%2d", &inputHour, &inputMinutes);

  minutesSinceMidnight = inputHour * 60 + inputMinutes;

  // Since we want the closest, not the next soonest, find the midpoint and compare to input.
  if (minutesSinceMidnight <= (d1 + d2) / 2) {
    // 8am departure, 10:16am arrival, explict newline prevents issues in some terminals, best
    // practice to remember them.
    printf("Closest departure time is 8:00 AM, arriving at 10:16 AM\n");
  } else if (minutesSinceMidnight <= (d2 + d3) / 2) {
    // 9:43am departure, 11:52am arrival
    printf("Closest departure time is 9:43 AM, arriving at 11:52 AM\n");
  } else if (minutesSinceMidnight <= (d3 + d4) / 2) {
    // 11:19am departure, 1:31pm arrival
    printf("Closest departure time is 11:19 AM, arriving at 1:31 PM\n");
  } else if (minutesSinceMidnight <= (d4 + d5) / 2) {
    // 12:47pm departure, 3pm arrival
    printf("Closest departure time is 12:47 PM, arriving at 3:00 PM\n");
  } else if (minutesSinceMidnight <= (d5 + d6) / 2) {
    // 2pm departure, 4:08pm arrival
    printf("Closest departure time is 2:00 PM, arriving at 4:08 PM\n");
  } else if (minutesSinceMidnight <= (d6 + d7) / 2) {
    // 3:45pm departure, 5:55pm arrival
    printf("Closest departure time is 3:45 PM, arriving at 5:55 PM\n");
  } else if (minutesSinceMidnight <= (d7 + d8) / 2) {
    // 7:00pm departure, 9:20 pm arrival
    printf("Closest departure time is 7:00 PM, arriving at 9:20 PM\n");
  } else {
    // 9:45pm departure, 11:58pm arrival
    printf("Closest departure time is 9:45 PM, arriving at 11:58 PM\n");
  }

  return 0;
}
