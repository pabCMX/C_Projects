#include <stdio.h>

int main(void)
{
  int itemNum, purchaseMonth, purchaseDay, purchaseYear;
  float itemPrice;

  printf("Enter item number: ");
  scanf("%d", &itemNum);
  printf("Enter unit price: ");
  scanf("%f", &itemPrice);
  printf("Enter purchase date: (mm/dd/yyyy): ");
  scanf("%d / %d / %d", &purchaseMonth, &purchaseDay, &purchaseYear);

  printf("------------------------------------\n");
  printf("Item\t\tUnit\t\tPurchase\n\t\tPrice\t\tDate\n");
  printf("%d\t\t$%6.2f\t\t%d/%d/%d", itemNum, itemPrice, purchaseMonth, purchaseDay, purchaseYear);

  return 0;

}