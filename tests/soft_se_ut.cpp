#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "secure-element.h"
#include "LoRaMac.h"
}

#include "nvm_images.hpp"
#include "string.h"

void run_stout_test(void);

TEST_GROUP(secure_element){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;


