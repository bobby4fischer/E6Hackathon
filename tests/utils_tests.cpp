#include <gtest/gtest.h>
#include "utils/string_utils.hpp"
#include <vector>

TEST(StringUtilsTest, TrimFunction) {
    EXPECT_EQ(aqe::utils::trim(" hello world "), "hello world");
    EXPECT_EQ(aqe::utils::trim("hello world "), "hello world");
    EXPECT_EQ(aqe::utils::trim(" hello world"), "hello world");
    EXPECT_EQ(aqe::utils::trim("hello world"), "hello world");
    EXPECT_EQ(aqe::utils::trim("\t\n hello world \r"), "hello world");
    EXPECT_EQ(aqe::utils::trim(""), "");
    EXPECT_EQ(aqe::utils::trim(" \t "), "");
}

TEST(StringUtilsTest, ToUpperFunction) {
    EXPECT_EQ(aqe::utils::toUpper("hello"), "HELLO");
    EXPECT_EQ(aqe::utils::toUpper("Hello World"), "HELLO WORLD");
    EXPECT_EQ(aqe::utils::toUpper("ALREADY UPPER"), "ALREADY UPPER");
    EXPECT_EQ(aqe::utils::toUpper(""), "");
    EXPECT_EQ(aqe::utils::toUpper("1a2b3c!@#"), "1A2B3C!@#");
}

TEST(StringUtilsTest, SplitCSVFunction) {
    std::vector<std::string> expected1 = {"one", "two", "three"};
    EXPECT_EQ(aqe::utils::splitCSV("one,two,three"), expected1);
    
    std::vector<std::string> expected2 = {"a", "b", "c"};
    EXPECT_EQ(aqe::utils::splitCSV(" a , b, c "), expected2);
    
    std::vector<std::string> expected3 = {"a", "", "c"};
    EXPECT_EQ(aqe::utils::splitCSV("a,,c"), expected3);
    
    std::vector<std::string> expected4 = {"a", "b", ""};
    EXPECT_EQ(aqe::utils::splitCSV("a,b,"), expected4);
}