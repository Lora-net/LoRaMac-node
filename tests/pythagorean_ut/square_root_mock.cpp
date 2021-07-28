#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include <stdio.h>
#include "square_root.h"
}


float SquareRoot_sqrt(float number)
{
    //printf("\nSquareRoot_sqrt gets called with parameter: %f.\n", number);

    return float(mock().actualCall(__func__)
        .withParameter("number", number)
        .returnDoubleValue());
}
