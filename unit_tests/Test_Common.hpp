#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h" 
#include "gmock/gmock-matchers.h" 
#include <cstdint>
#include <vector>
#include<string>

class TestBase : public ::testing::Test
{
protected:
    void PrintBuffer(std::string label, const std::vector<uint8_t>& buffer, uint32_t line_len = 32);
};

