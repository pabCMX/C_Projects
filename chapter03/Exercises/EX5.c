#include <stdio.h>

int main(void)
{
  int x, y, z, a, b, c, d, e, f, g, h, i, j, k, l, m;
  int firstCol, secondCol, thirdCol, fourthCol;
  int firstRow, secondRow, thirdRow, fourthRow;
  int forwardDiag, backwardDiag;

  printf("Enter the numbers 1 through 16 in any order, separated by spaces:\n");
  scanf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &x,&y, &z, &a, &b, &c, &d, &e, &f, &g, &h, &i, &j, &k, &l, &m);

  printf("\n%d\t%d\t%d\t%d\n", x, y, z, a);
  printf("%d\t%d\t%d\t%d\n", b, c, d, e);
  printf("%d\t%d\t%d\t%d\n", f, g, h, i);
  printf("%d\t%d\t%d\t%d\n\n", j, k, l, m);

  firstCol = x+b+f+j;
  secondCol = y+c+g+k;
  thirdCol = z+d+h+l;
  fourthCol = a+e+i+m;

  firstRow = x+y+z+a;
  secondRow = b+c+d+e;
  thirdRow = f+g+h+i;
  fourthRow = j+k+l+m;

  forwardDiag = a+d+g+j;
  backwardDiag = x+c+h+m;

  printf("Row Sums:\t%d\t%d\t%d\t%d\n", firstCol, secondCol, thirdCol, fourthCol);
  printf("Column Sums:\t%d\t%d\t%d\t%d\n", firstRow, secondRow, thirdRow, fourthRow);
  printf("Diagonal Sums:\t%d\t%d\n", backwardDiag, forwardDiag);
}