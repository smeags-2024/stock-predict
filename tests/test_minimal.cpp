#include <gtest/gtest.h>
#include <iostream>

// Minimal test to verify basic functionality
TEST(MinimalTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    std::cout << "Basic test passed!" << std::endl;
}

TEST(MinimalTest, StringTest) {
    std::string hello = "Hello";
    std::string world = "World";
    std::string result = hello + " " + world;
    EXPECT_EQ(result, "Hello World");
}

// Test C++20 features are working
TEST(MinimalTest, Cpp20Features) {
    // Test constexpr if
    constexpr bool flag = true;
    if constexpr (flag) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
}
