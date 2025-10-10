//  Name temps.c
//  Purpose: Compute Celsius degrees from Fahrenheit in a list

#include <stdio.h>

int main(void)
{
    float fahr, celsius;
    float lower, upper, step;

    lower = 0;
    upper = 100;
    step = 5;

    fahr = lower;
    while (fahr <= upper) {
        celsius = 5.0*(fahr-32.0)/9.0;
        //C = (5/9)(f-32)
        printf("%6.0f\t%6.1f\n", fahr, celsius);
        fahr = fahr + step;
    }

    return 0;
}