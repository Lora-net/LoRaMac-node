#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "print_utils.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
}

void run_stout_test(void);

TEST_GROUP(print_utils){
    void setup(){}

    void teardown(){
        mock().clear();
}
}
;

int x = 123456;
double y = 3.14;

TEST(print_utils, print_test)
{
    print_bytes(&x, sizeof(x));
    print_bytes(&y, sizeof(y));
}

IGNORE_TEST(print_utils, test_print)
{

    struct stat st;
    int bytesWritten = 0;

    // Redirect stdout
    freopen("redir.txt", "w", stdout);

    print_bytes(&x, sizeof(x));

    // assert checking
    stat("redir.txt", &st);
    bytesWritten = st.st_size;
    printf("%d", bytesWritten);

    CHECK_TRUE(bytesWritten > 0);
}
