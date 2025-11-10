//Convert base-10 int to base-8
#include <stdio.h>

int main (void){
  int input, tenThou, thousand, hundred, tensDigit, onesDigit;

  printf("Enter a number between 0 and 32767: ");
  scanf("%d", &input);

  onesDigit = input % 8;
  tensDigit = (input /= 8) % 8;
  hundred = (input /= 8 ) % 8;
  thousand = (input /= 8) % 8;
  tenThou = (input /= 8) % 8;

  printf("In ocat, your number is: %d%d%d%d%d", tenThou, thousand, hundred, tensDigit, onesDigit);

  return 0;
}
