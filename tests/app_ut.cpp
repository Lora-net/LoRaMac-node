#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "config.h"
#include "ublox.h"
#include "main.h"
#include "geofence.h"
#include "LoRaMac.h"
#include "nvmm.h"
}

#include "nvm_images.hpp"
#include "eeprom-board-mock.hpp"
#include "gps_mock_utils.hpp"

#include <string.h>

void prepare_n_position_mocks(int number_of_readings, int degrees_moved_per_shift);

TEST_GROUP(app){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;



