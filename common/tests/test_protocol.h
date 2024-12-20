#pragma once
#include <gtest/gtest.h>

class Test_BaseProtocol : public ::testing::Test
{
public:
protected:
    void SetUp() override;
    void TearDown() override;
};
