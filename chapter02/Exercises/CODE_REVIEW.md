# Chapter 2: C Fundamentals

Code review for Chapter 2 exercises, covering the basics of C programming including `printf`, `scanf`, variables, constants, and basic arithmetic operations.

---

## Exercise 1 – ASCII Art Checkmark

```c
// Print a checkmark to terminal.
#include <stdio.h>

int main(void) {
  printf("       *\n");
  printf("      * \n");
  printf("     *  \n");
  printf("*   *   \n");
  printf(" * *    \n");
  printf("  *     \n");
  return 0;
}
```

### Review

**Strengths:**

- Clear, descriptive comment stating the program's purpose
- Creative ASCII art that actually resembles a checkmark
- Proper use of `\n` escape sequences for newlines
- Clean `main(void)` signature following best practices
- Consistent indentation

**Suggestions:**

- The trailing spaces after some asterisks are unnecessary but harmless
- Could consolidate into a single `printf` with embedded `\n` characters, but multiple calls is fine for clarity

**Grade: A** – Simple but effective introduction to `printf`.

---

## Exercise 2 – Sphere Volume Calculator (Fixed Radius)

```c
// Compute the volume of a sphere of 10 meter radius.
#include <stdio.h>

int main(void) {
  const float PI = 3.14159265359;

  float volume;
  float radius = 10.0f;
  float radiusCubed;

  radiusCubed = radius * radius * radius;

  volume = (4.0f / 3.0f) * PI * radiusCubed;

  printf("Radius: %.1f, Volume: %.2f\n", radius, volume);

  return 0;
}
```

### Review

**Strengths:**

- **Excellent use of `const` for PI!** This is best practice
- Correct sphere volume formula: V = (4/3)πr³
- Good use of intermediate variable `radiusCubed` for clarity
- Proper use of `f` suffix on float literals (`10.0f`, `4.0f`, `3.0f`)
- Appropriate format specifiers (`%.1f`, `%.2f`)
- Proper trailing newline

**Suggestions:**

- Could use `math.h` and `M_PI` for more precision, but this is fine for Chapter 2

**Grade: A** – Solid demonstration of constants, floats, and arithmetic.

---

## Exercise 3 – Sphere Volume Calculator (User Input)

```c
// Refactor ex2.c to take user input for radius
#include <stdio.h>

int main(void) {
  const float PI = 3.14159265359;

  float volume, radius, radiusCubed;

  printf("Enter radius(meters): ");
  scanf("%f", &radius);

  radiusCubed = radius * radius * radius;
  volume      = (4.0f / 3.0f) * PI * radiusCubed;

  printf("Radius(meters): %.2f, Volume(meters cubed): %.2f\n", radius, volume);

  return 0;
}
```

### Review

**Strengths:**

- Good evolution from ex2 – comment documents the relationship
- Properly introduces `scanf` with `%f` format specifier
- Correctly uses address-of operator (`&radius`)
- Improved output message includes units (meters cubed)
- Consistent formatting and style with ex2
- Proper trailing newline

**Suggestions:**

- Consider adding a space after "radius" in prompt: `"Enter radius (meters): "`
- Negative radius input would produce incorrect results (volume can't be negative)

**Grade: A** – Clean introduction to `scanf` for user input.

---

## Exercise 4 – Sales Tax Calculator

```c
// Compute total with tax from user input subtotal
#include <stdio.h>

int main(void) {
  float subtotal, total, tax, taxPercent = 0.05f;

  printf("Enter an amount: $");
  scanf("%f", &subtotal);

  tax   = subtotal * taxPercent;
  total = subtotal + tax;

  printf("With tax added: $%.2f\n", total);

  return 0;
}
```

### Review

**Strengths:**

- Clean, practical example
- Good variable naming (`subtotal`, `total`, `tax`, `taxPercent`)
- **Nice alignment of assignments** (`tax =` and `total =`) – shows attention to code style
- Correct tax calculation logic
- Appropriate `%.2f` for currency display
- Proper trailing newline

**Suggestions:**

- Consider making `taxPercent` a `const` since it doesn't change
- Could also display the tax amount separately: `"Tax: $%.2f, Total: $%.2f"`

**Grade: A** – Practical real-world application with clean code.

---

## Exercise 5 – 5th Degree Polynomial (Direct Calculation)

```c
// Compute a 5th degree polynomial with user input x
#include <stdio.h>

int main(void) {
  float x, xFifthDegree, xFourthDegree, xThirdDegree, xSecondDegree, total;

  printf("Enter value for x: ");
  scanf("%f", &x);

  xFifthDegree  = x * x * x * x * x;
  xFourthDegree = x * x * x * x;
  xThirdDegree  = x * x * x;
  xSecondDegree = x * x;

  total =
      (3 * xFifthDegree) + (2 * xFourthDegree) - (5 * xThirdDegree) - xSecondDegree + (7 * x) - 6;
  printf("Value is: %f\n", total);

  return 0;
}
```

### Review

**Strengths:**

- Correct polynomial: 3x⁵ + 2x⁴ - 5x³ - x² + 7x - 6
- Clear variable names for each power
- **Good use of intermediate variables** to avoid repeated computation
- Properly parenthesized coefficients for clarity
- Line wrapping on long expression maintains readability
- Good alignment on power assignments

**Suggestions:**

- This approach requires 10 multiplications – ex6 will show a more efficient way
- Consider `%.2f` or similar for cleaner output instead of default `%f`

**Grade: A** – Solid foundation before learning Horner's Rule.

---

## Exercise 6 – 5th Degree Polynomial (Horner's Rule)

```c
// Refactor ex5.c to use Horner's Rule for polynomials.
#include <stdio.h>

int main(void) {
  float x, total;

  printf("Enter value for x: ");
  scanf("%f", &x);

  total = ((((3 * x + 2) * x - 5) * x - 1) * x + 7) * x - 6;
  printf("Value is: %f\n", total);

  return 0;
}
```

### Review

**Strengths:**

- **Excellent implementation of Horner's Rule!** Reduces 10 multiplications to 5
- Comment properly documents the optimization
- Same polynomial, more efficient evaluation
- Correct nested structure: 3x⁵ + 2x⁴ - 5x³ - x² + 7x - 6 = ((((3x + 2)x - 5)x - 1)x + 7)x - 6
- Dramatically cleaner code – only 2 variables needed
- Proper trailing newline

**Suggestions:**

- A comment showing the factored form would be educational:
  ```c
  // 3x^5 + 2x^4 - 5x^3 - x^2 + 7x - 6
  // = ((((3x + 2)x - 5)x - 1)x + 7)x - 6
  ```

**Grade: A+** – Elegant application of an important numerical technique.

---

## Exercise 7 – Bill Change Calculator

```c
// Compute dollar change for user input total.
#include <stdio.h>

int main(void) {
  int billTwenty, billTen, billFive, billOne, change;

  printf("Enter change amount: $");
  scanf("%d", &change);

  billTwenty = change / 20;
  billTen    = (change - (billTwenty * 20)) / 10;
  billFive   = (change - (billTwenty * 20 + billTen * 10)) / 5;
  billOne    = change - (billTwenty * 20 + billTen * 10 + billFive * 5);

  printf("$20 bills:\t%d\n", billTwenty);
  printf("$10 bills:\t%d\n", billTen);
  printf("$5 bills:\t%d\n", billFive);
  printf("$1 bills:\t%d\n", billOne);

  return 0;
}
```

### Review

**Strengths:**

- **Correct greedy algorithm** for making change
- Proper use of integer division for bill counts
- Uses `int` type appropriately (whole dollar amounts)
- Clean output with tabs for alignment
- Proper trailing newlines

**Suggestions:**

- Could simplify by modifying `change` after each bill calculation:
  ```c
  billTwenty = change / 20;
  change %= 20;
  billTen = change / 10;
  change %= 10;
  // etc.
  ```
  But current approach is correct and shows understanding of the problem
- Negative input would produce incorrect results

**Grade: A** – Solid greedy algorithm implementation.

---

## Exercise 8 – Loan Payment Calculator

```c
// Compute first 3 monthly payments with interest based on user loan data.
#include <stdio.h>

int main(void) {
  float principal, interest, payment, balance;
  printf("Enter loan amount: $");
  scanf("%f", &principal);
  printf("Enter interest rate (%%): ");
  scanf("%f", &interest);
  interest = interest / 100;
  printf("Enter payment: $");
  scanf("%f", &payment);

  balance = (principal * (1 + (interest / 12.0f))) - payment;
  printf("Balance after first payment: $%.2f\n", balance);

  principal = balance;
  balance   = (principal * (1 + (interest / 12.0f))) - payment;
  printf("Balance after second payment: $%.2f\n", balance);

  principal = balance;
  balance   = (principal * (1 + (interest / 12.0f))) - payment;
  printf("Balance after third payment: $%.2f\n", balance);

  return 0;
}
```

### Review

**Strengths:**

- **Correct compound interest formula!** Balance = Principal × (1 + r/12) - Payment
- Proper conversion from percentage to decimal (`interest / 100`)
- Good use of `%%` escape sequence in prompt to display percent sign
- Multiple payments show understanding of iterative financial calculations
- Currency formatted properly with `%.2f`
- Clear, descriptive output messages
- Proper trailing newlines

**Suggestions:**

- This pattern of repeating code screams for a loop (which you'll learn later!)
- Consider showing the interest charged each month for transparency:
  ```c
  float monthlyInterest = principal * (interest / 12.0f);
  ```
- Variable naming is excellent

**Grade: A** – Excellent real-world financial calculation with proper formula.

---

# Summary

## Overall Assessment

Excellent work on Chapter 2, demonstrating solid understanding of:
- Basic I/O with `printf` and `scanf`
- Variable types (`int`, `float`) and constants
- Arithmetic operations and precedence
- Format specifiers for different types
- Good code organization and style

| Exercise | Grade | Highlights |
| -------- | ----- | ---------- |
| 1 | A | Clean ASCII art with proper escape sequences |
| 2 | A | Proper use of constants and float arithmetic |
| 3 | A | Good introduction to scanf |
| 4 | A | Practical tax calculation with clean style |
| 5 | A | Clear polynomial evaluation foundation |
| 6 | A+ | Excellent Horner's Rule implementation |
| 7 | A | Correct greedy change algorithm |
| 8 | A | Real-world compound interest calculation |

**Chapter Average: A**

## Key Strengths

- Consistent use of `const` for unchanging values (PI)
- Good variable naming throughout
- Proper use of float literals with `f` suffix
- Clean code formatting and alignment
- Comments that document purpose and relationships between exercises
- Proper trailing newlines on all output

## Areas for Future Growth

- Consider edge cases (negative inputs, zero values)
- Some comments have minor typos
- As you learn loops, the repetitive code in ex8 can be refactored

## Highlights

- **Exercise 6** – Horner's Rule is an elegant optimization that shows deeper understanding
- **Exercise 8** – Compound interest is a practical, real-world application
- **Code Style** – Consistent alignment of assignments shows attention to detail


