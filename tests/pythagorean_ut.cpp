#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "pythagorean.h"
}


TEST_GROUP(Pythagorean)
{
    void setup() {
    }


    void teardown() {
        mock().clear();
    }

};


TEST(Pythagorean, simpleTest)
{
    // arrange
    mock().expectOneCall("SquareRoot_sqrt")
          .withParameter("number", 25.0)
          .andReturnValue(5.0f);

    // act
    float hypotenuse = Pythagorean_hypotenuse(3, 4);

    // assert
    mock().checkExpectations();
    CHECK_EQUAL(5.0f, hypotenuse);
}


TEST(Pythagorean, BlackBoxTest)
{
    // arrange
    mock().expectOneCall("SquareRoot_sqrt")
          .withParameter("number", 100.0)
          .andReturnValue(100.0f); // intentionally set the result as 100.0 instead of 10.0.

    // act
    float hypotenuse = Pythagorean_hypotenuse(6, 8);

    // assert
    mock().checkExpectations();
    CHECK_EQUAL(100.0f, hypotenuse);
}


