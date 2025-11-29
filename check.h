// guard/check.h
#ifndef GUARD_CHECK_H
#define GUARD_CHECK_H

#include "env.h"
#include "location.h"
#include "macro.h"

#include <string>
#include <sstream>

static inline std::string guard_location_part(struct guard_location loc)
{
    std::ostringstream os;
    os << "Error checked in location \n"
       << "\tline:" << loc.line << "\n"
       << "\tfile:" << loc.file << "\n"
       << "\tfunc:" << loc.func << "\n";
    return os.str();
}

// Базовый CHECK: выражение должно быть истинным
#define GUARD_CHECK(expr)                                                       \
    do                                                                          \
    {                                                                           \
        if (!(expr))                                                            \
        {                                                                       \
            GUARD_CURRENT_LOCATION(loc);                                        \
            std::ostringstream _guard_os;                                       \
            _guard_os << guard_location_part(loc)                               \
                      << "\tcond:" << GUARD_STRINGIFY(expr) << "\n\f";          \
            GUARD_CHECK_ENV_RAISE_SET(_guard_os.str());                         \
            GUARD_CHECK_ENV_RAISE_IMPL();                                       \
        }                                                                       \
    } while (0)

// Равенство: a == b
#define GUARD_CHECK_EQ(a, b)                                                    \
    do                                                                          \
    {                                                                           \
        const auto& _guard_a = (a);                                             \
        const auto& _guard_b = (b);                                             \
        if (!(_guard_a == _guard_b))                                            \
        {                                                                       \
            GUARD_CURRENT_LOCATION(loc);                                        \
            std::ostringstream _guard_os;                                       \
            _guard_os << guard_location_part(loc)                               \
                      << "\tcond:"                                              \
                      << GUARD_STRINGIFY(a) " == " GUARD_STRINGIFY(b) << "\n"   \
                      << "\tleft: " << _guard_a << "\n"                         \
                      << "\tright: " << _guard_b << "\n";                       \
            GUARD_CHECK_ENV_RAISE_SET(_guard_os.str());                         \
            GUARD_CHECK_ENV_RAISE_IMPL();                                       \
        }                                                                       \
    } while (0)

// Неравенство: a != b
#define GUARD_CHECK_NEQ(a, b)                                                   \
    do                                                                          \
    {                                                                           \
        const auto& _guard_a = (a);                                             \
        const auto& _guard_b = (b);                                             \
        if (!(_guard_a != _guard_b))                                            \
        {                                                                       \
            GUARD_CURRENT_LOCATION(loc);                                        \
            std::ostringstream _guard_os;                                       \
            _guard_os << guard_location_part(loc)                               \
                      << "\tcond:"                                              \
                      << GUARD_STRINGIFY(a) " != " GUARD_STRINGIFY(b) << "\n"   \
                      << "\tleft: " << _guard_a << "\n"                         \
                      << "\tright: " << _guard_b << "\n";                       \
            GUARD_CHECK_ENV_RAISE_SET(_guard_os.str());                         \
            GUARD_CHECK_ENV_RAISE_IMPL();                                       \
        }                                                                       \
    } while (0)

// Меньше: a < b
#define GUARD_CHECK_LT(a, b)                                                    \
    do                                                                          \
    {                                                                           \
        const auto& _guard_a = (a);                                             \
        const auto& _guard_b = (b);                                             \
        if (!(_guard_a < _guard_b))                                             \
        {                                                                       \
            GUARD_CURRENT_LOCATION(loc);                                        \
            std::ostringstream _guard_os;                                       \
            _guard_os << guard_location_part(loc)                               \
                      << "\tcond:"                                              \
                      << GUARD_STRINGIFY(a) " < " GUARD_STRINGIFY(b) << "\n"    \
                      << "\tleft: " << _guard_a << "\n"                         \
                      << "\tright: " << _guard_b << "\n";                       \
            GUARD_CHECK_ENV_RAISE_SET(_guard_os.str());                         \
            GUARD_CHECK_ENV_RAISE_IMPL();                                       \
        }                                                                       \
    } while (0)

// Больше: a > b
#define GUARD_CHECK_GT(a, b)                                                    \
    do                                                                          \
    {                                                                           \
        const auto& _guard_a = (a);                                             \
        const auto& _guard_b = (b);                                             \
        if (!(_guard_a > _guard_b))                                             \
        {                                                                       \
            GUARD_CURRENT_LOCATION(loc);                                        \
            std::ostringstream _guard_os;                                       \
            _guard_os << guard_location_part(loc)                               \
                      << "\tcond:"                                              \
                      << GUARD_STRINGIFY(a) " > " GUARD_STRINGIFY(b) << "\n"    \
                      << "\tleft: " << _guard_a << "\n"                         \
                      << "\tright: " << _guard_b << "\n";                       \
            GUARD_CHECK_ENV_RAISE_SET(_guard_os.str());                         \
            GUARD_CHECK_ENV_RAISE_IMPL();                                       \
        }                                                                       \
    } while (0)

#endif // GUARD_CHECK_H
