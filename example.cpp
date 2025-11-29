#include "check.h"
#include "guard.h"
#include <stdexcept>

TEST_CASE("simple arithmetic")
{
    CHECK_EQ(2 + 2, 4);
    CHECK_LT(1, 10);
}

TEST_CASE("exceptions")
{
    CHECK_THROWS(throw std::runtime_error("oops"));
    CHECK_THROWS_AS(throw std::runtime_error("oops"), std::runtime_error);
    CHECK_NOTHROW(int x = 42; (void)x;);
}

TEST_CASE("timeout demo")
{
    CHECK_TIMEOUT(
        []
        {
            for (volatile int i = 0; i < 1000000; ++i)
                ;
        }(),
        1000);
}

TEST_CASE("failed test: simple arithmetic")
{
    CHECK_EQ(2 + 2, 4);
    CHECK_LT(20, 10);
}

GUARD_TEST_MAIN();