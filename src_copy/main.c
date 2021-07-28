#include <stdio.h>
#include "pythagorean.h"

int main(void)
{
    int x = 30;
    int y = 40;
    float hypotenuse = Pythagorean_hypotenuse(x, y);

    printf("Hypotenuse of a right triangle with 2 sides as (%d, %d) is %0.1f.\n", x, y, hypotenuse);

    return 0;
}