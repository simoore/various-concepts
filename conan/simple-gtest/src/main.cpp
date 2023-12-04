#include <iostream>
#include "gtest/gtest.h"

class TestFixture : public ::testing::Test {
protected:
    void SetUp(){
    std::cout << "SetUp()" << std::endl;
    }

    void TearDown(){
    std::cout << "TearDown()" << std::endl;
    }
};



TEST_F (TestFixture, shouldCompile) {
    std::cout << "shouldCompile" << std::endl;
    ASSERT_TRUE(true); // works, maybe optimized out?
    ASSERT_TRUE("hi" == "hallo"); // undefined reference

}

int main(int argc, char **argv) {
    std::cout << "Starting tests" << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}