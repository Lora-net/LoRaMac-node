#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "bsp.h"
}

TEST_GROUP(bsp_ut){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

TEST(bsp_ut, bsp_init)
{
    BSP_sensor_Init();
}

TEST(bsp_ut, bsp_sensor_read)
{
    BSP_sensor_Init();
    BSP_sensor_Read();
}

TEST(bsp_ut, manage_incoming_instruction_from_ground)
{
    uint8_t test_instruction[] = {0x4e, 0xb3, 0x07, 0x00, 0x75, 0xaf, 0x07, 0x00};
    manage_incoming_instruction(test_instruction);
}
