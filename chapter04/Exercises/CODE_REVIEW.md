# Chapter 4: Expressions and Operators

Code review for Chapter 4 exercises, covering expressions and major operators including arithmetic, assignment, and compound assignment operators.

---

## Exercise 1 – Two-Digit Number Reversal

```c
// Enter a two digit number and return the number reversed.
#include <stdio.h>

int main(void) {
  int input, tensDigit, onesDigit;

  printf("Enter a two-digit number: ");
  scanf("%d", &input);

  onesDigit = input % 10;
  tensDigit = input / 10;

  printf("The reversal is: %d%d\n", onesDigit, tensDigit);

  return 0;
}
```

### Review

**Strengths:**

- Perfect use of modulo and integer division
- Good comment describing the program
- Clear variable names
- Elegant, minimal solution
- Proper trailing newline

**Suggestions:**

- Consider edge cases: What if user enters a single-digit number? The output would be correct but potentially confusing (e.g., input `5` outputs `50`).

**Grade: A** – Clean demonstration of operators.

---

## Exercise 2 – Three-Digit Number Reversal (Mathematical)

```c
// Extending Ex1 to handle 3 digit numbers
#include <stdio.h>

int main(void) {
  int input, hundreds, tensDigit, onesDigit;

  printf("Input a 3 digit number: ");
  scanf("%d", &input);

  hundreds  = input / 100;
  onesDigit = input % 10;
  tensDigit = (input % 100) / 10;

  printf("The reversal is: %d%d%d\n", onesDigit, tensDigit, hundreds);

  return 0;
}
```

### Review

**Strengths:**

- Correct extension of the previous approach
- Good logical progression from ex1
- Comment links back to previous exercise
- Simplified ones digit extraction (`input % 10` instead of `(input % 100) % 10`)
- Proper trailing newline

**Suggestions:**

- Nice that it handles the mathematical decomposition explicitly

**Grade: A** – Solid extension with clean digit extraction.

---

## Exercise 3 – Three-Digit Reversal (scanf Width)

```c
// Ex2 without using math, instead input splits.
#include <stdio.h>

int main(void) {
  int hundreds, tensDigit, onesDigit;

  printf("Enter a three-digit number: ");
  scanf("%1d%1d%1d", &hundreds, &tensDigit, &onesDigit);

  printf("The reversal is: %d%d%d\n", onesDigit, tensDigit, hundreds);

  return 0;
}
```

### Review

**Strengths:**

- **Excellent use of scanf width specifiers!** This is creative and shows deep understanding of formatted input.
- Clean and elegant solution
- Good comment explaining the different approach
- Proper trailing newline

**Suggestions:**

- This approach won't work correctly if the user enters spaces between digits
- Demonstrates good knowledge of Chapter 3 material (formatted I/O) combined with Chapter 4

**Grade: A+** – Creative application of format specifiers.

---

## Exercise 4 – Decimal to Octal Converter

```c
// Convert base-10 int to base-8
#include <stdio.h>

int main(void) {
  int input, tenThou, thousand, hundred, tensDigit, onesDigit;

  printf("Enter a number between 0 and 32767: ");
  scanf("%d", &input);

  onesDigit = input % 8;
  tensDigit = (input /= 8) % 8;
  hundred   = (input /= 8) % 8;
  thousand  = (input /= 8) % 8;
  tenThou   = (input /= 8) % 8;

  printf("In octal, your number is: %d%d%d%d%d\n", tenThou, thousand, hundred, tensDigit, onesDigit);
  // We could've just used printf("Octal: %o", input) to get the conversion automatically...

  return 0;
}
```

### Review

**Strengths:**

- **Excellent use of compound assignment operators (`/=`)!** This is exactly what Chapter 4 teaches.
- Correct base conversion algorithm
- Good range specified in prompt (32767 = 77777 in octal, max 5 digits)
- Proper trailing newline
- Great self-aware comment about the `%o` format specifier – shows you understand there's a shortcut but deliberately implemented the algorithm

**Suggestions:**

- Variable names suggest decimal positions but represent octal positions – consider `octal1, octal2...` or just `d1, d2...`

**Grade: A** – Great operator usage with educational insight.

---

## Exercise 5 – UPC Check Digit Calculator

```c
// upc.c updated for single user input.
#include <stdio.h>

int main(void) {
  int d, i1, i2, i3, i4, i5, j1, j2, j3, j4, j5, first_sum, second_sum, total;

  printf("Enter the first 11 digits of a UPC: ");
  scanf("%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d", &d, &i1, &i2, &i3, &i4, &i5, &j1, &j2, &j3, &j4, &j5);

  first_sum  = d + i2 + i4 + j1 + j3 + j5;
  second_sum = i1 + i3 + i5 + j2 + j4;
  total      = 3 * first_sum + second_sum;

  printf("Check digit: %d\n", 9 - ((total - 1) % 10));

  return 0;
}
```

### Review

**Strengths:**

- Great use of scanf width specifiers for individual digits
- Correct UPC check digit algorithm
- Clean grouping of odd/even position sums
- Good comment referencing the source

**Suggestions:**

- The check digit formula `9 - ((total - 1) % 10)` is clever but could use a comment explaining why this works (equivalent to `(10 - (total % 10)) % 10`)
- Variable naming `i` and `j` groups are logical but could be more descriptive

**Grade: A** – Correct implementation with good operator usage.

---

## Exercise 6 – EAN Check Digit Calculator

```c
// Update upc.c to calculate EANs instead
#include <stdio.h>

int main(void) {
  int i1, i2, i3, i4, i5, i6, j1, j2, j3, j4, j5, j6, first_sum, second_sum, total;

  printf("Enter the first 12 digits of a UPC: ");
  scanf("%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d%1d", &i1, &i2, &i3, &i4, &i5, &i6, &j1, &j2, &j3, &j4,
        &j5, &j6);

  first_sum  = i2 + i4 + i6 + j2 + j4 + j6;
  second_sum = i1 + i3 + i5 + j1 + j3 + j5;
  total      = 3 * first_sum + second_sum;

  printf("Check digit: %d\n", 9 - ((total - 1) % 10));

  return 0;
}
```

### Review

**Strengths:**

- Good extension from UPC to EAN
- Correctly adjusted algorithm for EAN-13 (even positions × 3 vs odd positions × 3 in UPC)
- Comment links to previous work
- Consistent style with ex5

**Suggestions:**

- The prompt says "UPC" but should say "EAN" for accuracy
- Same suggestion about commenting the check digit formula

**Grade: A** – Correct adaptation; minor prompt text issue.

---

# Summary

## Overall Assessment

Excellent work on Chapter 4, demonstrating mastery of:
- Modulo and integer division for digit extraction
- Compound assignment operators (`/=`)
- Expression evaluation order
- Combining multiple concepts creatively

| Exercise | Grade | Highlights |
| -------- | ----- | ---------- |
| 1 | A | Clean 2-digit reversal |
| 2 | A | Extended to 3 digits with clean extraction |
| 3 | A+ | Creative scanf width approach |
| 4 | A | Compound assignment for base conversion |
| 5 | A | UPC check digit algorithm |
| 6 | A | EAN adaptation |

**Chapter Average: A**

## Key Strengths

- Excellent use of compound assignment operators
- Creative combination of format specifiers with arithmetic
- Understanding of base conversion algorithms
- Good code documentation and comments

## Areas for Future Growth

- Consider adding comments for complex mathematical formulas
- Watch for prompt/output text accuracy

