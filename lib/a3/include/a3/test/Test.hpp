#pragma once

#include <gtest/gtest.h>

namespace a3::test {

class A3Test : public ::testing::Test {
    protected:
        virtual void SetUp() override;
        virtual void TearDown() override;
    public:
        A3Test();
        ~A3Test();
};

}