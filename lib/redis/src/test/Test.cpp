#include "a3/test/Test.hpp"

namespace a3::test {

A3Test::A3Test() { }
A3Test::~A3Test() { }

void A3Test::SetUp() {
}
void A3Test::TearDown() {
}

TEST_F(A3Test, test) {
    EXPECT_TRUE(true);
}

} // namespace a3::test