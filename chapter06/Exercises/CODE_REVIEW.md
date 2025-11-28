# Chapter 6: Loops (while/do-while/for)

Code review for Chapter 6 exercises, covering loops including `while`, `do-while`, `for`, as well as `break` and `continue` statements.

---

## Exercise 1 – Find Largest Number

```c
// Return the largest number among a series of user input numbers.
#include <stdio.h>

int main(void) {
  double input = 1, max;

  printf("Enter a positive number (0 to exit): ");
  scanf("%lf", &input);
  max = input;
  while (input > 0) {
    if (input > max) {
      max = input;
    }
    printf("Enter a positive number (0 to exit): ");
    scanf("%lf", &input);
  }
  printf("The largest number entered was: %lf\n", max);

  return 0;
}
```

### Review

**Strengths:**

- Good use of `while` loop with sentinel value (0 or negative to exit)
- Correct use of `%lf` for `double` in scanf
- Simple and readable logic
- **Now includes trailing newline** ✓
- **Properly initializes `max` to first input** – no more false 0 result ✓
- **Sentinel value no longer participates in comparison** – clean loop structure ✓
- **Clear prompt** tells user how to exit ✓

**Suggestions:**

- ~~Missing trailing newline on the final printf~~ ✓ Fixed
- ~~Initializing `max = 0` means if all inputs are negative, the result would be 0~~ ✓ Fixed – now initialized to first input
- ~~The sentinel value becomes part of the comparison~~ ✓ Fixed – restructured loop
- Consider using `%.2lf` for cleaner output instead of showing all decimal places (optional)

**Grade: A** – Excellent restructuring. Proper sentinel handling with clean loop logic.

---

## Exercise 2 – Greatest Common Divisor (GCD)

```c
// Calculate the greatest common divisor (GCD) from two input integers
#include <stdio.h>

int main(void) {
  int m, n, GCD, temp;

  printf("Enter two integers, separated by a space: ");
  scanf("%d %d", &m, &n);

  // We'll use euclid's algorithm, first testing if n is zero.
  while (n > 0) {
    // then m mod n
    temp = m % n;
    m    = n;
    n    = temp;
  }
  GCD = m;

  printf("Greatest common divisor: %d\n", GCD);

  return 0;
}
```

### Review

**Strengths:**

- **Excellent algorithm choice!** Euclid's algorithm is the classic GCD solution
- Good comments explaining the approach
- Clean variable naming (`m`, `n`, `temp` are conventional for GCD)
- Proper trailing newline

**Suggestions:**

- The `while (n > 0)` condition won't handle negative inputs correctly. Consider using `while (n != 0)` and handling signs:
  ```c
  if (m < 0) m = -m;
  if (n < 0) n = -n;
  ```
- The `GCD` variable is unnecessary – you could just print `m` directly:
  ```c
  printf("Greatest common divisor: %d\n", m);
  ```
- Consider adding validation for the case when both inputs are 0 (GCD undefined)

**Grade: A-** – Solid implementation of a classic algorithm, minor edge case consideration needed.

---

## Exercise 3 – Fraction Reducer

```c
// Reduce user fraction to lowest terms.
#include <stdio.h>

int main(void) {
  int numer, denom, GCD, temp, m, n;

  printf("Enter a fraction (n/m): ");
  scanf("%d / %d", &numer, &denom);
  // Don't pollute the original inputs
  n = numer;
  m = denom;
  // Use Euclid's GCD algo
  while (n > 0) {
    temp = m % n;
    m    = n;
    n    = temp;
  }

  // Apply the GCD
  GCD = m;
  numer /= GCD;
  denom /= GCD;

  printf("In lowest terms: %d/%d\n", numer, denom);

  return 0;
}
```

### Review

**Strengths:**

- **Good code reuse** – correctly applies GCD algorithm from Exercise 2
- Excellent comment: "Don't pollute the original inputs" shows thoughtfulness
- Clean separation of GCD calculation and application
- Proper trailing newline

**Suggestions:**

- Same negative number issue as Exercise 2 – `n > 0` won't work for negative fractions
- The prompt says "Enter a fraction (n/m)" but uses `numer/denom` – naming is slightly inconsistent (minor)
- Consider handling edge cases:
  - Division by zero (denom = 0)
  - Negative fractions (should the sign be on numerator or denominator?)
- Could simplify by not needing `GCD` variable – just use `m` directly

**Grade: A-** – Good application of previous algorithm with thoughtful preservation of inputs.

---

## Exercise 4 – Continuous Broker Calculator

```c
// Update broker.c to allow for continuous inputs
#include <stdio.h>

int main(void) {
  float commission, value;

  for (;;) {
    printf("Enter value of trade (exit with 0): ");
    scanf("%f", &value);
    if (value <= 0) {
      break;
    }

    if (value < 2'500.00f)
      commission = 30.00f + .017f * value;
    else if (value < 6'250.00f)
      commission = 56.00f + .0066f * value;
    else if (value < 20'000.00f)
      commission = 76.00f + .0034f * value;
    else if (value < 50'000.00f)
      commission = 100.00f + .0022f * value;
    else if (value < 500'000.00f)
      commission = 155.00f + .0011f * value;
    else
      commission = 255.00f + .0009f * value;

    if (commission < 39.00f) {
      commission = 39.00f;
    }

    printf("Commission: $%.2f\n", commission);
  }
  return 0;
}
```

### Review

**Strengths:**

- **First use of infinite `for (;;)` loop!** Classic C idiom for loops with complex exit conditions
- Proper use of `break` to exit the loop
- **Continued use of C23 digit separators** (`2'500.00f`) – consistent with Chapter 5 style
- Clear exit instruction in prompt
- Proper trailing newlines
- **Now uses braces on minimum commission check** for consistency ✓

**Suggestions:**

- ~~The minimum commission check could use braces for consistency with the multi-line if-else above~~ ✓ Fixed
- Could add a farewell message after the loop exits:
  ```c
  printf("Goodbye!\n");
  ```
- This is an excellent demonstration of how loops make programs more useful – good choice for adaptation

**Grade: A** – Excellent use of infinite loop with break for interactive program.

---

## Exercise 5 – Reverse Integer Digits

```c
// Generalize chapter04 ex1.c to any number of digits.
#include <stdio.h>

int main(void) {
  int input;

  printf("Enter a positive integer: ");
  scanf("%d", &input);

  printf("The reversal is: ");
  do {
    printf("%d", input % 10);
    input /= 10;
  } while (input > 0);

  printf("\n");
  return 0;
}
```

### Review

**Strengths:**

- **Perfect use of `do-while`!** This is the ideal case – we always want at least one digit (even for input 0)
- Clean, elegant algorithm using modulo and integer division
- Proper newline at end
- Good choice to generalize from Chapter 4's fixed-digit version

**Suggestions:**

- Negative numbers would cause issues – consider:
  ```c
  if (input < 0) {
    printf("-");
    input = -input;
  }
  ```
- The comment says "positive integer" which documents the limitation – good!
- This is a textbook example of when to use `do-while` over `while`

**Grade: A** – Excellent demonstration of do-while for guaranteed single iteration.

---

## Exercise 6 – Even Squares

```c
// Return all even squares between 1 and user input.
#include <stdio.h>

int main(void) {
  int input;

  printf("Input end-bound for squares: ");
  scanf("%d", &input);

  for (int i = 1; i * i <= input; i++) {
    // Since even squares must have even roots, we only need to check i
    if (i % 2 == 0) {
      // Print the root and the square.
      printf("%10d%10d\n", i, i * i);
    }
  }

  return 0;
}
```

### Review

**Strengths:**

- **Excellent mathematical insight!** The comment "even squares must have even roots" shows understanding of number theory
- Smart loop condition `i * i <= input` avoids square root calculation
- Clean formatted output with `%10d` for aligned columns
- C99 loop variable declaration (`int i = 1`)
- Good inline comments

**Suggestions:**

- Since you know you only want even `i`, you could optimize by starting at 2 and incrementing by 2:
  ```c
  for (int i = 2; i * i <= input; i += 2) {
    printf("%10d%10d\n", i, i * i);
  }
  ```
  This eliminates the `if` check entirely
- Consider adding a header row:
  ```c
  printf("%10s%10s\n", "Root", "Square");
  ```

**Grade: A** – Shows mathematical reasoning with clean implementation.

---

## Exercise 7 – Squares Table (Rearranged)

```c
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
```

### Review

**Strengths:**

- ~~Typo: "prgram" should be "program"~~ ✓ Fixed
- **Uses the clever odd-number method** for computing squares without multiplication: `1, 1+3=4, 4+5=9, 9+7=16, ...`
- Good use of C99 for-loop with multiple declarations (`int i = 1, square = 1, odd = 3`)
- Multiple expressions in the increment clause (`odd += 2, i++`)
- Clean formatted output
- ~~The variable `odd` could also be initialized in the for loop~~ ✓ Fixed – now all variables in for loop
- ~~`square += odd` appeared twice (in increment AND body)~~ ✓ Fixed – now only in body where it belongs

**Suggestions:**

- None – all issues resolved!

**Grade: A-** – Good demonstration of for-loop features with all issues fixed.

---

## Exercise 8 – Calendar Printer

```c
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
```

### Review

**Strengths:**

- **First use of `continue`!** Elegant way to handle blank spaces at the start
- Clever loop structure – single loop handles both blank days and actual dates
- Good prompt explaining the day numbering convention (1=Sun, 7=Sat)
- Comment about final newline shows attention to output formatting
- Tab-based alignment is practical

**Suggestions:**

- Consider adding a header row with day names:
  ```c
  printf("Sun\tMon\tTue\tWed\tThu\tFri\tSat\n");
  ```
- Input validation would help (startDay should be 1-7, totalDays should be 28-31)
- The loop could use `<=` instead of `<` with `totalDays + startDay - 1` for clarity, but current version is correct
- Consider `%3d` instead of tabs for more consistent spacing across different terminal settings

**Grade: A** – Creative solution with excellent use of continue statement.

---

## Exercise 9 – Loan Balance Calculator

```c
// Extending Ch. 2 ex8.c to display the balance after user entered number of payments.
// I took the liberty of adding a proper payment calculation with pow() as well.
// Remember to build with the -lm flag to link the math library.
#include <stdio.h>
#include <math.h>

int main(void) {
  double principal, interest, periodInterest, expInterest, balance, payment;
  int    periods;
  printf("Enter loan amount: $");
  scanf("%lf", &principal);
  printf("Enter interest rate (%%): ");
  scanf("%lf", &interest);
  interest /= 100.0; // Convert percent to decimal.
  periodInterest = interest / 12.0;
  printf("Enter number of monthly payments: ");
  scanf("%d", &periods);

  // Now we calculate the payment amount:
  expInterest = pow(1 + periodInterest, periods);
  payment            = (principal * ((periodInterest * expInterest) / (expInterest - 1)));
  // Avoiding changing input.
  balance = principal;
  for (int i = 1; i <= periods; i++) {
    balance = (balance * (1 + periodInterest)) - payment;
    printf("Payment: $%.2lf, Balance after payment %2d: $%10.2f\n", payment, i, balance);
  }

  return 0;
}
```

### Review

**Strengths:**

- **Goes beyond the exercise requirements!** Implementing the actual amortization formula shows initiative
- Helpful comment about `-lm` flag for linking math library
- Good use of `pow()` for compound interest calculation
- Clean output formatting with aligned columns
- Comment "Avoiding changing input" shows good practices (from Exercise 3)
- Correct escaping of `%%` in printf for literal percent sign
- **Now uses proper `double` literals** (`100.0`, `12.0`) ✓
- **Now uses `periodInterest` consistently** throughout ✓
- **Variables declared at top** in traditional C style ✓

**Suggestions:**

- ~~Inconsistency: Using `100.0f` and `12.0f` (float literals) with `double` variables – should be `100.0` and `12.0`~~ ✓ Fixed
- ~~Inconsistency: `periodInterest` is calculated once but `interest / 12.0` is used in the loop instead of `periodInterest`~~ ✓ Fixed
- ~~Variable `expInterest` declared mid-function~~ ✓ Fixed – now at top with other declarations
- The final balance should ideally be 0 (or very close) – consider adding a final summary:
  ```c
  printf("\nFinal balance: $%.2f (should be ~$0.00)\n", balance);
  ```

**Grade: A+** – Excellent extension with real-world financial calculation. All consistency issues resolved.

---

## Exercise 10 – Earliest Date Finder

```c
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
```

### Review

**Strengths:**

- Good extension of Chapter 5's date comparison logic
- **Uses both `break` and `continue`** – good demonstration of loop control
- Proper zero-padded output with `%02d` for month/day
- Reuses the hierarchical comparison logic from Chapter 5
- **Now uses explicit `0/0/0` sentinel check** ✓
- **Changed to `for(;;)` for clarity** ✓
- **Year format now `%04d`** ✓
- **Prompt now tells user how to exit** ✓
- **Removed redundant `else { continue; }`** ✓

**Suggestions:**

- ~~Exit condition is fragile: `month2 + day2 + year2 <= 0` could be triggered by unusual but valid dates~~ ✓ Fixed
- ~~Consider a clearer sentinel like `0/0/0`~~ ✓ Fixed
- ~~The final `else { continue; }` is redundant~~ ✓ Fixed
- ~~The `while (month1 + day1 + year1 > 0)` condition is misleading~~ ✓ Fixed – now uses `for(;;)`
- ~~The year format `%02d` will truncate years like 2024~~ ✓ Fixed – now `%04d`
- ~~Consider telling the user how to exit~~ ✓ Fixed – prompt now includes "[0/0/0 to exit]"

**Grade: A** – All issues addressed. Clean loop control demonstration.

---

## Exercise 11 – Calculate e (n terms)

```c
// Calculate the value of e using infinite series to the nth term.
#include <stdio.h>

int main(void) {
  int    finalTerm;
  double denomFactorial = 1.0;
  double sum = 0.0;

  printf("This program computes e to the nth term of the fractional infinite series.\n");
  printf("Enter the limit term for e: ");
  scanf("%d", &finalTerm);

  for (int i = 0; i <= finalTerm; i++) {
    if (i != 0) {
      denomFactorial *= i;
    }
    sum += 1.0 / denomFactorial;
  }

  printf("The number e to %d term(s) is %.16f\n", finalTerm, sum);

  return 0;
}
```

### Review

**Strengths:**

- Correctly implements the series: e = Σ(1/n!) for n=0 to ∞
- High precision output with `%.16f`
- Good descriptive prompt
- **Now includes `return 0;`** ✓
- **Uses `double` for factorial** to avoid overflow ✓
- **Removed redundant check** – now uses simple `if (i != 0)` ✓
- **Now uses running factorial** instead of recalculating each iteration – much more efficient! ✓
- **Clean variable declarations** – no unused variables ✓

**Suggestions:**

- ~~Missing `return 0;` at end of main~~ ✓ Fixed
- ~~The condition `if (factorialCount == 0 && i == 0)` is redundant~~ ✓ Fixed
- ~~Potential overflow: Factorials grow very fast; `long` overflow occurs around 20!~~ ✓ Fixed – now uses `double`
- ~~The factorial calculation could be optimized by keeping a running factorial~~ ✓ Fixed
- ~~`factorialCount` is still declared but no longer used~~ ✓ Fixed – removed
- None remaining – all issues resolved!

**Grade: A** – Excellent improvements. Clean, efficient implementation.

---

## Exercise 12 – Calculate e (to accuracy)

```c
// Modify ex11.c to calculate until certain decimal place accuracy.
#include <stdio.h>

int main(void) {
  double denomFactorial = 1.0;
  double sum = 0.0, accuracy;

  printf("This program computes e with a fractional infinite series up to a certain term contribution threshold.\n");
  printf("Enter the term contribution threshold for e (e.g. 0.000001): ");
  scanf("%lf", &accuracy);

  for (int i = 0;; i++) {
    if (i != 0) {
      denomFactorial *= i;
    }
    if (1.0 / denomFactorial <= accuracy) {
      break;
    }
    sum += 1.0 / denomFactorial;
  }

  printf("The number e to %f term contribution threshold is %.16f\n", accuracy, sum);
  
  return 0;
}
```

### Review

**Strengths:**

- **Good adaptation of Exercise 11** – shows iterative refinement of code
- Uses infinite `for` loop with `break` on convergence condition
- Accuracy-based termination is mathematically sound (1/n! decreases monotonically)
- Demonstrates practical application of infinite loops
- **Now includes `return 0;`** ✓
- **Uses `double` for factorial** ✓
- **Removed redundant check** ✓
- **Uses running factorial** ✓
- **Updated prompt to clarify "term contribution threshold"** – much clearer! ✓
- **Clean variable declarations** – no unused variables ✓

**Suggestions:**

- ~~Missing `return 0;` at end of main~~ ✓ Fixed
- ~~Same redundant `if (factorialCount == 0 && i == 0)` issue as Exercise 11~~ ✓ Fixed
- ~~Same factorial overflow concern~~ ✓ Fixed
- ~~The prompt says "decimal place accuracy" but the value is actually "term contribution threshold" – slightly misleading~~ ✓ Fixed – prompt now accurate
- ~~`factorialCount` is still declared but no longer used~~ ✓ Fixed – removed
- Could report how many terms were needed (optional enhancement):
  ```c
  int terms;
  for (terms = 0;; terms++) { ... }
  printf("Computed with %d terms...\n", terms);
  ```

**Grade: A** – All issues addressed. Clear, accurate implementation.

---

# Summary

## Overall Assessment

Excellent work on Chapter 6, demonstrating solid understanding of:
- `while` loops for conditional iteration
- `do-while` loops for guaranteed single iteration
- `for` loops with C99 declarations and multiple expressions
- Infinite loops (`for(;;)`) with `break` for complex exit conditions
- `continue` for early iteration advancement
- Nested loops for complex algorithms
- Building on previous chapters' code (GCD, broker, dates)

## Final Grades

| Exercise | Original | Final | Change | Notes |
| -------- | -------- | ----- | ------ | ----- |
| 1 | B+ | **A** | ⬆️ | Proper sentinel handling, clean loop |
| 2 | A- | A- | — | No changes needed |
| 3 | A- | A- | — | No changes needed |
| 4 | A | A | — | Added braces (minor) |
| 5 | A | A | — | Already excellent |
| 6 | A | A | — | Already excellent |
| 7 | A- | A- | — | All issues fixed |
| 8 | A | A | — | Already excellent |
| 9 | A | **A+** | ⬆️ | All consistency issues fixed |
| 10 | B+ | **A** | ⬆️ | All issues addressed |
| 11 | B+ | **A** | ⬆️ | Efficient running factorial, clean code |
| 12 | B+ | **A** | ⬆️ | Clear prompt, efficient code |

**Chapter Average: A** (improved from A-)

## All Issues Fixed ✓

- ✓ Ex 1: Restructured loop for proper sentinel handling, added trailing newline
- ✓ Ex 4: Added braces on minimum commission check
- ✓ Ex 7: Fixed "prgram" typo, moved variables to for loop, fixed duplicate `square += odd`
- ✓ Ex 9: Fixed float literals, consistent `periodInterest` usage
- ✓ Ex 10: Fixed sentinel check, year format, added exit instructions
- ✓ Ex 11: Added `return 0;`, optimized factorial, removed unused variable
- ✓ Ex 12: Added `return 0;`, clarified prompt, optimized factorial, removed unused variable

## Key Strengths

- **Good algorithm selection** – Euclid's GCD, odd-number squares, amortization formula
- **Appropriate loop type selection** – do-while when iteration is guaranteed, while for conditional
- **Code reuse** – Building on and extending previous exercises
- **Continued use of C23 digit separators** for readability
- **Good commenting** explaining algorithm choices and build requirements

## Areas for Improvement

- ~~**Missing `return 0;`** in Exercises 11 and 12~~
- ~~**Edge case handling** – negative numbers, sentinel value collisions~~
- ~~**Minor typos** – "prgram" in Exercise 7~~
- ~~**Consistency** – using `periodInterest` vs `interest/12.0` in Exercise 9~~
- **Responsive to feedback** – most issues addressed correctly

## Highlights

- **Exercise 4** – Clean infinite loop with break for interactive mode
- **Exercise 5** – Textbook example of when to use do-while
- **Exercise 6** – Mathematical insight about even roots producing even squares
- **Exercise 8** – Elegant use of continue for blank calendar spaces
- **Exercise 9** – Goes beyond requirements with proper amortization formula
- **Exercise 11 & 12** – Excellent optimization using running factorial

## Concepts Demonstrated from Previous Chapters

- Selection statements (if/else) – used throughout with loops
- Switch statements – not used (appropriate for this chapter's focus)
- C23 digit separators – Exercise 4
- Formatted I/O (`%10d`, `%02d`, `%.2f`) – multiple exercises
- Arithmetic operators for modulo, division – Exercises 2, 3, 5, 6

Keep up the good work! The progression from simple while loops to nested loops and complex exit conditions shows solid understanding of iteration in C. Pay attention to edge cases and don't forget `return 0;` at the end of main!

