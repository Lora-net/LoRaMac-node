#include "pythagorean.h"
#include "square_root.h"


float Pythagorean_hypotenuse(float x, float y)
{
    float hypotenuse = SquareRoot_sqrt(x*x + y*y);
    return hypotenuse;
}
