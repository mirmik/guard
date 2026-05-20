#include "guard_c.h"

GUARD_C_TEST(simple_arithmetic)
{
    GUARD_C_CHECK_EQ_INT(4, 2 + 2);
    GUARD_C_CHECK(10 > 1);
    return 0;
}

GUARD_C_TEST(strings)
{
    GUARD_C_REQUIRE_STREQ("guard", "guard");
    GUARD_C_CHECK_STREQ(NULL, NULL);
    return 0;
}

GUARD_C_TEST(floating_point)
{
    GUARD_C_CHECK_NEAR_DOUBLE(1.0, 0.1 + 0.2 + 0.7, 1e-12);
    return 0;
}

int main(int argc, char** argv)
{
    GUARD_C_BEGIN_ARGS(argc, argv);
    GUARD_C_RUN(simple_arithmetic);
    GUARD_C_RUN(strings);
    GUARD_C_RUN(floating_point);
    return GUARD_C_END();
}
