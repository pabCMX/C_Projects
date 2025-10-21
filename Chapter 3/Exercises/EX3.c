#include <stdio.h>

int main(void)
{
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