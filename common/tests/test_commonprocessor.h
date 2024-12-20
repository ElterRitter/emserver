#pragma once

#include "commonprocessor.h"

#include <gtest/gtest.h>

#include <memory>

class Test_CommonProcessor : public ::testing::Test
{
public:
    Test_CommonProcessor();

protected:
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<CommonProcessor> m_proc;
};

