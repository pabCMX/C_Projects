#include <stdio.h>

int main (void)
{
  int billTwenty, billTen, billFive, billOne, change;

  printf("Enter change amount: $");
  scanf("%d", &change);

  billTwenty = change / 20;
  billTen = (change - (billTwenty * 20)) / 10;
  billFive = (change - (billTwenty * 20 + billTen * 10)) / 5;
  billOne = change - (billTwenty * 20 + billTen * 10 + billFive * 5);

  printf("$20 bills:\t%d\n", billTwenty);
  printf("$10 bills:\t%d\n", billTen);
  printf("$5 bills:\t%d\n", billFive);
  printf("$1 bills:\t%d\n", billOne);


  return 0;
}