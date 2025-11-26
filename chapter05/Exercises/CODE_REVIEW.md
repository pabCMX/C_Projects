# Chapter 5: Selection Statements (if/switch)

Code review for Chapter 5 exercises, covering logical expressions and selection statements including `if`, `else if`, `else`, and `switch`.

---

## Exercise 1 – Digit Counter

```c
// Calculates digits in a given number. We can assume the number is no bigger than 4 digits.
#include <stdio.h>

int main(void) {
  int userNum;
  int count;

  printf("Enter a number: ");
  scanf("%d", &userNum);

  // To make sure negatives don't break it, we'll throw the sign away
  if (userNum < 0) {
    userNum = -userNum;
  }

  if (userNum <= 9) {
    count = 1;
  } else if (userNum <= 99) {
    count = 2;
  } else if (userNum <= 999) {
    count = 3;
  } else if (userNum <= 9999) {
    count = 4;
  } else {
    count = 5;
  }

  printf("The number %d has %d digits.\n", userNum, count);

  return 0;
}
```

### Review

**Strengths:**

- Good use of cascading if-else structure
- Correct boundary values (9, 99, 999, 9999)
- Clean formatting with braces on all branches
- **Excellent handling of negative numbers** with clear comment explaining the approach
- Updated comment noting the assumption about number size
- Proper trailing newline

**Suggestions:**

- Numbers >= 100000 are assumed to have 5 digits (the comment now documents this limitation)

**Grade: A** – Good logic with proper edge case handling.

---

## Exercise 2 – 24-Hour to 12-Hour Time Converter

```c
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
```

### Review

**Strengths:**

- **Excellent comment noting the pitfalls!** Shows thoughtful analysis
- Correctly handles all four cases: 0:xx, 1-11:xx, 12:xx, 13-23:xx
- Good use of `%.2d` (equivalent to `%02d`) for zero-padded output
- Width specifiers in scanf for proper parsing
- Proper trailing newlines on all output branches

**Suggestions:**

- Could consolidate the two PM cases:
  ```c
  if (userHours >= 12) {
    convertedHours = (userHours == 12) ? 12 : userHours - 12;
    printf("... PM\n");
  }
  ```
  But the explicit handling is very clear and correct as-is.

**Grade: A** – Thorough handling of edge cases with good documentation.

---

## Exercise 3 – Broker Commission Comparison

```c
// Update to broker.c to calculate from shares and share price
// and compare to rival broker commission fees.
#include <stdio.h>

int main(void) {
  int   shareCount;
  float commission, rivalCommission, sharePrice, tradeValue;

  printf("Enter share price: ");
  scanf("%f", &sharePrice);
  printf("Enter number of shares: ");
  scanf("%d", &shareCount);
  tradeValue = sharePrice * shareCount;

  if (tradeValue < 2'500.00f)
    commission = 30.00f + .017f * tradeValue;
  else if (tradeValue < 6'250.00f)
    commission = 56.00f + .0066f * tradeValue;
  else if (tradeValue < 20'000.00f)
    commission = 76.00f + .0034f * tradeValue;
  else if (tradeValue < 50'000.00f)
    commission = 100.00f + .0022f * tradeValue;
  else if (tradeValue < 500'000.00f)
    commission = 155.00f + .0011f * tradeValue;
  else
    commission = 255.00f + .0009f * tradeValue;

  if (commission < 39.00f) {
    commission = 39.00f;
  }

  if (shareCount < 2000) {
    rivalCommission = 33.00f + 0.03f * shareCount;
  } else {
    rivalCommission = 33.00f + 0.02f * shareCount;
  }
  printf("Our Commission: $%.2f\nTheir Commission: $%.2f\n", commission, rivalCommission);

  return 0;
}
```

### Review

**Strengths:**

- **Creative use of C23 digit separators (`2'500.00f`)!** Shows awareness of modern C features
- Correctly implements tiered commission structure
- Minimum commission check is properly placed after calculation
- Good comparison with rival pricing
- Proper trailing newline
- Consistent brace usage on minimum commission check

**Suggestions:**

- The digit separators are a C23 feature – ensure your compiler supports it (most modern compilers do with `-std=c23`)
- Consider adding a recommendation to the output:
  ```c
  if (commission < rivalCommission)
    printf("\nWe offer the better deal!\n");
  ```

**Grade: A** – Sophisticated real-world calculation with modern C features.

---

## Exercise 4 – Beaufort Scale Classifier

```c
// Computes Beaufort scale from given wind speed in knots
#include <stdio.h>

int main(void) {
  float windSpeed;

  printf("Enter your wind speed (in knots): ");
  scanf("%f", &windSpeed);

  if (windSpeed < 1) {
    printf("Calm.\n");
  } else if (windSpeed <= 3) {
    printf("Very light air.\n");
  } else if (windSpeed <= 6) {
    printf("Light breeze.\n");
  } else if (windSpeed <= 10) {
    printf("Gentle breeze.\n");
  } else if (windSpeed <= 16) {
    printf("Moderate breeze.\n");
  } else if (windSpeed <= 21) {
    printf("Fresh or cooling breeze.\n");
  } else if (windSpeed <= 27) {
    printf("Strong breeze.\n");
  } else if (windSpeed <= 33) {
    printf("Near Gale, High wind.\n");
  } else if (windSpeed <= 40) {
    printf("Gale Winds.\n");
  } else if (windSpeed <= 47) {
    printf("Severe Gale.\n");
  } else if (windSpeed <= 55) {
    printf("Storm, Whole Gale.\n");
  } else if (windSpeed <= 63) {
    printf("Violent Storm\n");
  } else {
    printf("Hurricane force winds.\n");
  }

  return 0;
}
```

### Review

**Strengths:**

- Complete implementation of the Beaufort scale
- Correct boundary values matching the official scale
- Clean if-else ladder structure
- Good descriptive output messages
- Proper trailing newlines on all output

**Suggestions:**

- Consider also printing the Beaufort number:
  ```c
  printf("Force 0: Calm.\n");
  ```
- Negative wind speeds should probably be handled (though physically impossible)
- This is a good candidate for a switch statement on the Beaufort number, but the if-else with float comparisons is appropriate given the input type

**Grade: A** – Complete and accurate implementation.

---

## Exercise 5 – Income Tax Calculator

```c
// Calculates a resident's income tax given a total taxable income.
#include <stdio.h>

int main(void) {
  float income, tax;

  printf("Enter your yearly taxable income: ");
  scanf("%f", &income);

  if (income < 750) {
    tax = income * 0.01f;

  } else if (income < 2'250) {
    tax = ((income - 750) * 0.02f) + 7.5f;

  } else if (income < 3'750) {
    tax = ((income - 2'250) * 0.03f) + 7.5f + 30.0f;

  } else if (income < 5'250) {
    tax = ((income - 3'750) * 0.04f) + 7.5f + 30.0f + 45.0f;

  } else if (income < 7'000) {
    tax = ((income - 5'250) * 0.05f) + 7.5f + 30.0f + 45.0f + 60.0f;
  } else {
    tax = ((income - 7'000) * 0.06f) + 7.5f + 30.0f + 45.0f + 60.0f + 87.5f;
  }

  printf("The tax due on your income of $%5.2f is: $%5.2f\n", income, tax);

  return 0;
}
```

### Review

**Strengths:**

- Correct marginal tax bracket implementation
- Good understanding of how marginal rates work (taxing each bracket separately)
- The cumulative base amounts (7.5, 30.0, 45.0, etc.) show understanding of bracket calculations
- Proper float literals throughout
- **Good use of C23 digit separators** in bracket comparisons for readability
- Proper trailing newline

**Suggestions:**

- Consider using named constants for the bracket boundaries and cumulative taxes:
  ```c
  const float BRACKET1_TAX = 7.5f;   // Tax on first $750
  const float BRACKET2_TAX = 30.0f;  // Tax on $750-$2250
  // etc.
  ```
- The `%5.2f` format may truncate larger incomes – consider `%.2f` for flexibility
- Negative income handling could be added

**Grade: A** – Excellent demonstration of marginal tax calculation.

---

## Exercise 6 – UPC Validator

```c
// Update to upc.c to state whether UPC given is valid.
#include <stdio.h>

int main(void) {
  int d, i1, i2, i3, i4, i5, j1, j2, j3, j4, j5, checksum, first_sum, second_sum, total,
      computedCheck;

  printf("Enter a complete UPC: ");
  scanf("%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d", &d, &i1, &i2, &i3, &i4, &i5, &j1, &j2, &j3, &j4,
        &j5, &checksum);

  first_sum     = d + i2 + i4 + j1 + j3 + j5;
  second_sum    = i1 + i3 + i5 + j2 + j4;
  total         = 3 * first_sum + second_sum;
  computedCheck = 9 - ((total - 1) % 10);

  if (computedCheck == checksum) {
    printf("The UPC is valid.\n");
  } else {
    printf("The UPC is invalid. Checksum should be %d, got %d\n", computedCheck, checksum);
  }

  return 0;
}
```

### Review

**Strengths:**

- Good evolution from Chapter 4's UPC calculator
- Proper validation logic with computed vs. provided checksum
- Helpful error message showing expected value
- **Correctly displays `computedCheck`** in error message
- Proper trailing newlines

**Suggestions:**

- None – the error message now correctly shows the computed check digit

**Grade: A** – Correct validation with clear error reporting.

---

## Exercise 7 – Find Min and Max of Four Numbers

```c
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
```

### Review

**Strengths:**

- **Clever tournament-style algorithm!** This is an efficient approach
- Good use of comments explaining the logic
- The pairing approach minimizes comparisons (4 comparisons vs 6 for naive approach)
- Well-organized swap logic
- Proper trailing newline

**Suggestions:**

- Initialize `max` and `min` to 0 is unnecessary since they're always assigned
- The swap pattern is good – in later chapters you'll learn this can be done without a temp variable using XOR or compound expressions
- Consider adding a comment explaining the tournament approach:
  ```c
  // Ensure i1 >= i2 (larger of first pair in i1)
  ```

**Grade: A** – Efficient algorithm showing algorithmic thinking.

---

## Exercise 8 – Flight Departure Finder

```c
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
```

### Review

**Strengths:**

- **Excellent algorithm design!** Using midpoints to find the closest departure is mathematically sound
- Converting everything to minutes-since-midnight is the right approach
- Great comments explaining the data and algorithm choice
- Proper newlines on all output
- Self-documenting comment about newline best practices

**Suggestions:**

- The arrival times could also be computed and stored as variables for consistency
- Consider handling edge cases:
  - Times before the first departure (currently handled correctly)
  - Times very late at night (should they wrap to the next day's 8 AM?)
- The comments are excellent – this is one of the best-documented exercises

**Grade: A+** – Excellent problem decomposition, algorithm design, and documentation.

---

# Summary

## Overall Assessment

Outstanding work on Chapter 5, demonstrating mastery of:
- Cascading if-else structures
- Proper boundary condition handling
- Edge case consideration (negative numbers, midnight, etc.)
- Algorithm design for complex problems
- Modern C features (C23 digit separators)

| Exercise | Grade | Highlights |
| -------- | ----- | ---------- |
| 1 | A | Digit counting with negative handling |
| 2 | A | Time conversion with all edge cases |
| 3 | A | Tiered commission with C23 features |
| 4 | A | Complete Beaufort scale |
| 5 | A | Marginal tax calculation |
| 6 | A | UPC validation with clear errors |
| 7 | A | Efficient tournament min/max |
| 8 | A+ | Excellent flight finder algorithm |

**Chapter Average: A**

## Key Strengths

- Thorough edge case handling (negative numbers, boundary times)
- Excellent algorithm design (tournament, midpoint)
- Good use of modern C23 features
- Outstanding code documentation and comments
- Consistent proper output formatting

## Highlights

- **Exercise 1** – Proper negative number handling with clear comment
- **Exercise 3** – Creative use of C23 digit separators for readability
- **Exercise 7** – Efficient tournament-style min/max algorithm
- **Exercise 8** – Excellent documentation and midpoint algorithm design

Keep up the excellent work! The code quality, problem-solving approach, and attention to edge cases shows strong understanding of C fundamentals and selection statements.

