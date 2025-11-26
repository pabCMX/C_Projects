# Chapter 3: Formatted Input/Output

Code review for Chapter 3 exercises, covering `printf` and `scanf` formatting including format specifiers, field widths, precision, and conversion specifications.

---

## Exercise 1 – Receipt Formatter

```c
// Print formatted 'receipt' from user purchase info.
#include <stdio.h>

int main(void) {
  int   itemNum, purchaseMonth, purchaseDay, purchaseYear;
  float itemPrice;

  printf("Enter item number: ");
  scanf("%d", &itemNum);
  printf("Enter unit price: ");
  scanf("%f", &itemPrice);
  printf("Enter purchase date: (mm/dd/yyyy): ");
  scanf("%d / %d / %d", &purchaseMonth, &purchaseDay, &purchaseYear);

  printf("------------------------------------\n");
  printf("Item\t\tUnit\t\tPurchase\n\t\tPrice\t\tDate\n");
  printf("%d\t\t$%6.2f\t\t%d/%d/%d\n", itemNum, itemPrice, purchaseMonth, purchaseDay,
         purchaseYear);

  return 0;
}
```

### Review

**Strengths:**

- **Good use of scanf with spaces** to skip the `/` separators in date input
- Clean tabular output using `\t` characters
- **Nice field width specifier `%6.2f`** for price alignment
- Creative header with two-line layout
- Multiple `scanf` calls demonstrate proper sequential input
- Proper trailing newline

**Suggestions:**

- Consider using `%02d/%02d/%04d` for the date to maintain consistent digit widths (e.g., `01/05/2024` instead of `1/5/2024`)
- Tab-based alignment can be fragile across different terminal widths; fixed field widths would be more robust
- The separator line length (36 chars) is a nice touch

**Grade: A** – Good demonstration of formatted output with tabs and field widths.

---

## Exercise 2 – ISBN Parser

```c
// Split user ISBN into component parts for display.
#include <stdio.h>

int main(void) {
  int gs1Prefix, groupId, publisherCode, itemNum, checksum;

  printf("Enter ISBN (with dashes): ");
  scanf("%d-%d-%d-%d-%d", &gs1Prefix, &groupId, &publisherCode, &itemNum, &checksum);

  printf("GS1 Prefix: %d\n", gs1Prefix);
  printf("Group Identifier: %d\n", groupId);
  printf("Publisher Code: %d\n", publisherCode);
  printf("Item Number: %d\n", itemNum);
  printf("Check Digit: %d\n", checksum);

  return 0;
}
```

### Review

**Strengths:**

- **Excellent use of scanf pattern matching** with `-` as literal separators
- Correct identification of ISBN-13 components
- **Good domain knowledge** – proper naming of ISBN fields (GS1 prefix, Group ID, Publisher Code, etc.)
- Clean, educational output showing each component
- Proper trailing newlines

**Suggestions:**

- ISBNs can have leading zeros in some fields (e.g., publisher code `0123`) which would be lost with `%d`; consider `%s` with character arrays for complete accuracy, but this is fine for demonstration
- The check digit in ISBN-13 is calculated differently than variable name `checksum` might suggest, but naming is acceptable

**Grade: A** – Creative application of scanf format strings with real-world data.

---

## Exercise 3 – Phone Number Formatter

```c
// Transform user phone number to dotted format.
#include <stdio.h>

int main(void) {
  int areaCode, firstTriplet, secondQuartet;

  printf("Enter phone number [(xxx) xxx-xxxx]: ");
  scanf("(%d) %d-%d", &areaCode, &firstTriplet, &secondQuartet);

  printf("You entered: %03d.%03d.%04d\n", areaCode, firstTriplet, secondQuartet);

  return 0;
}
```

### Review

**Strengths:**

- **Excellent scanf pattern matching** with parentheses, space, and hyphen literals
- **Perfect use of zero-padded output** with `%03d` and `%04d`!
- Clean transformation from one format to another
- Good variable naming describing each phone number segment
- Proper trailing newline

**Suggestions:**

- Minor: `secondQuartet` should be `lastQuartet` or `suffix` – it's 4 digits, not a "second" anything
- The format hint in the prompt `[(xxx) xxx-xxxx]` is helpful for users

**Grade: A+** – Excellent demonstration of both scanf pattern matching and printf zero-padding.

---

## Exercise 4 – Magic Square Calculator

```c
// Take user input numbers and compute magic square sums.
#include <stdio.h>

int main(void) {
  int x, y, z, a, b, c, d, e, f, g, h, i, j, k, l, m;
  int firstCol, secondCol, thirdCol, fourthCol;
  int firstRow, secondRow, thirdRow, fourthRow;
  int forwardDiag, backwardDiag;

  printf("Enter the numbers 1 through 16 in any order, separated by spaces:\n");
  scanf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &x, &y, &z, &a, &b, &c, &d, &e, &f, &g,
        &h, &i, &j, &k, &l, &m);

  // Printing grid
  printf("\n%d\t%d\t%d\t%d\n", x, y, z, a);
  printf("%d\t%d\t%d\t%d\n", b, c, d, e);
  printf("%d\t%d\t%d\t%d\n", f, g, h, i);
  printf("%d\t%d\t%d\t%d\n\n", j, k, l, m);

  firstCol  = x + b + f + j;
  secondCol = y + c + g + k;
  thirdCol  = z + d + h + l;
  fourthCol = a + e + i + m;

  firstRow  = x + y + z + a;
  secondRow = b + c + d + e;
  thirdRow  = f + g + h + i;
  fourthRow = j + k + l + m;

  forwardDiag  = a + d + g + j;
  backwardDiag = x + c + h + m;

  printf("Row Sums:\t%d\t%d\t%d\t%d\n", firstRow, secondRow, thirdRow, fourthRow);
  printf("Column Sums:\t%d\t%d\t%d\t%d\n", firstCol, secondCol, thirdCol, fourthCol);
  printf("Diagonal Sums:\t%d\t%d\n", backwardDiag, forwardDiag);

  return 0;
}
```

### Review

**Strengths:**

- **Complex scanf handling 16 integers!** Shows confidence with formatted input
- Grid display is visually clear with tabs
- Correct calculation of all sums (rows, columns, diagonals)
- Good organization with grouped variable declarations
- Comment explains the grid printing section
- Educational – teaches about magic squares (all sums should equal 34 for 1-16)
- Clean output formatting with aligned labels

**Suggestions:**

- Variable names `x, y, z, a, b...` are hard to follow; consider `n1, n2...n16` or using arrays (in later chapters)
- Could verify it's a valid magic square by checking if all sums equal 34
- Fixed field widths (`%3d`) would align the grid better for two-digit numbers

**Grade: A** – Impressive handling of many inputs with good organization.

---

## Exercise 5 – Fraction Adder

```c
// Refactor addFrac.c to take a single user input
#include <stdio.h>

int main(void) {
  int num1, denom1, num2, denom2, resultNum, resultDenom;

  printf("Enter two fractions separated by a plus sign: ");
  scanf("%d / %d + %d / %d", &num1, &denom1, &num2, &denom2);

  resultNum   = num1 * denom2 + num2 * denom1;
  resultDenom = denom1 * denom2;

  printf("The sum is %d/%d\n", resultNum, resultDenom);

  return 0;
}
```

### Review

**Strengths:**

- **Excellent scanf pattern matching** with `/` and `+` literals
- Correct fraction addition algorithm: a/b + c/d = (ad + bc) / bd
- Clean, mathematical variable names
- Single-line input is user-friendly
- Good comment referencing the source file
- Proper trailing newline

**Suggestions:**

- The result is not reduced to lowest terms (e.g., 4/6 instead of 2/3) – requires GCD algorithm from later chapters
- Consider handling division by zero (denom1 or denom2 = 0)
- Input format could be shown in prompt: `"Enter (e.g., 1/2 + 3/4): "`

**Grade: A** – Clean implementation with elegant scanf pattern.

---

## Exercise 6 – Date Format Converter

```c
// Transform user input mm/dd/yyyy to yyyy/mm/dd
#include <stdio.h>

int main(void) {
  int year, month, day;

  printf("Enter a date (mm/dd/yyyy): ");
  scanf("%d / %d / %d", &month, &day, &year);

  printf("You entered the date: %d/%d/%d\n", year, month, day);

  return 0;
}
```

### Review

**Strengths:**

- Clean date format transformation (US to ISO-like format)
- Good use of scanf with `/` literal separators
- Clear prompt showing expected input format
- Simple and effective solution
- Proper trailing newline

**Suggestions:**

- Consider zero-padded output for ISO 8601 compliance: `%04d/%02d/%02d` would produce `2024/01/05` instead of `2024/1/5`
- Output message could be clearer: `"ISO format: %04d-%02d-%02d\n"` (ISO uses hyphens)
- No validation for valid dates (month 1-12, day 1-31, etc.)

**Grade: A** – Simple but effective format conversion.

---

# Summary

## Overall Assessment

Strong work on Chapter 3, demonstrating solid mastery of:
- Complex scanf format strings with literal characters
- Multiple format specifiers in single scanf/printf calls
- Field widths and precision specifiers
- Zero-padding for numeric output
- Practical applications of formatted I/O

| Exercise | Grade | Highlights |
| -------- | ----- | ---------- |
| 1 | A | Tabular receipt with field widths |
| 2 | A | ISBN parsing with pattern matching |
| 3 | A+ | Phone format conversion with zero-padding |
| 4 | A | Complex 16-integer magic square input |
| 5 | A | Fraction addition with scanf patterns |
| 6 | A | Date format transformation |

**Chapter Average: A**

## Key Strengths

- Excellent grasp of scanf pattern matching with literal characters (/, -, +, parentheses)
- Good use of field width and precision specifiers
- Proper zero-padding where appropriate (`%03d`, `%04d`)
- Real-world applications (ISBN, phone numbers, receipts, dates)
- Clean code organization and commenting
- Consistent proper trailing newlines

## Areas for Future Growth

- Consider zero-padding dates for consistency
- Fixed field widths can improve alignment over tabs
- Input validation will come with conditional statements (Chapter 5)
- Array handling will simplify exercises like the magic square (later chapters)

## Highlights

- **Exercise 3** – Perfect use of zero-padded output for phone number formatting
- **Exercise 4** – Impressive handling of 16 inputs with correct magic square calculations
- **Exercise 5** – Elegant scanf pattern for fraction input

The exercises show excellent practical application of formatted I/O concepts with real-world data formats (ISBN, phone numbers, dates, receipts). This provides a strong foundation for the expression and operator work in Chapter 4.


