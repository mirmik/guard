// guard/check.h
#ifndef GUARD_CHECK_H
#define GUARD_CHECK_H

#include "env.h"
#include "location.h"
#include "macro.h"

#include <sstream>
#include <string>

static inline std::string guard_location_part(struct guard_location loc)
{
    std::ostringstream os;
    os << "Error checked in location \n"
       << "\tline:" << loc.line << "\n"
       << "\tfile:" << loc.file << "\n"
       << "\tfunc:" << loc.func << "\n";
    return os.str();
}

// Базовый CHECK: выражение должно быть истинным (мягкий, не рвёт тест)
#define GUARD_CHECK(expr)                                                      \
    do                                                                         \
    {                                                                          \
        if (!(expr))                                                           \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc)                              \
                      << "\tcond:" << GUARD_STRINGIFY(expr) << "\n\f";         \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
        }                                                                      \
    } while (0)

// Базовый REQUIRE: выражение должно быть истинным (жёсткий, рвёт тест)
#define GUARD_REQUIRE(expr)                                                    \
    do                                                                         \
    {                                                                          \
        if (!(expr))                                                           \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc)                              \
                      << "\tcond:" << GUARD_STRINGIFY(expr) << "\n\f";         \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
            GUARD_CHECK_ENV_RAISE_IMPL();                                      \
        }                                                                      \
    } while (0)

// CHECK_FALSE: выражение должно быть ложным (мягкий)
#define GUARD_CHECK_FALSE(expr)                                                \
    do                                                                         \
    {                                                                          \
        if (expr)                                                              \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc)                              \
                      << "\tcond: !" << GUARD_STRINGIFY(expr) << "\n\f";       \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
        }                                                                      \
    } while (0)

// REQUIRE_FALSE: выражение должно быть ложным (жёсткий)
#define GUARD_REQUIRE_FALSE(expr)                                              \
    do                                                                         \
    {                                                                          \
        if (expr)                                                              \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc)                              \
                      << "\tcond: !" << GUARD_STRINGIFY(expr) << "\n\f";       \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
            GUARD_CHECK_ENV_RAISE_IMPL();                                      \
        }                                                                      \
    } while (0)

// Равенство: a == b (мягкий)
#define GUARD_CHECK_EQ(a, b)                                                   \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a == _guard_b))                                           \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " == " GUARD_STRINGIFY(b) << "\n"  \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
        }                                                                      \
    } while (0)

// Равенство: a == b (жёсткий)
#define GUARD_REQUIRE_EQ(a, b)                                                 \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a == _guard_b))                                           \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " == " GUARD_STRINGIFY(b) << "\n"  \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
            GUARD_CHECK_ENV_RAISE_IMPL();                                      \
        }                                                                      \
    } while (0)

// Неравенство: a != b (мягкий)
#define GUARD_CHECK_NEQ(a, b)                                                  \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a != _guard_b))                                           \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " != " GUARD_STRINGIFY(b) << "\n"  \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
        }                                                                      \
    } while (0)

// Неравенство: a != b (жёсткий)
#define GUARD_REQUIRE_NEQ(a, b)                                                \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a != _guard_b))                                           \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " != " GUARD_STRINGIFY(b) << "\n"  \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
            GUARD_CHECK_ENV_RAISE_IMPL();                                      \
        }                                                                      \
    } while (0)

// Меньше: a < b (мягкий)
#define GUARD_CHECK_LT(a, b)                                                   \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a < _guard_b))                                            \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " < " GUARD_STRINGIFY(b) << "\n"   \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
        }                                                                      \
    } while (0)

// Меньше: a < b (жёсткий)
#define GUARD_REQUIRE_LT(a, b)                                                 \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a < _guard_b))                                            \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " < " GUARD_STRINGIFY(b) << "\n"   \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
            GUARD_CHECK_ENV_RAISE_IMPL();                                      \
        }                                                                      \
    } while (0)

// Больше: a > b (мягкий)
#define GUARD_CHECK_GT(a, b)                                                   \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a > _guard_b))                                            \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " > " GUARD_STRINGIFY(b) << "\n"   \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
        }                                                                      \
    } while (0)

// Больше: a > b (жёсткий)
#define GUARD_REQUIRE_GT(a, b)                                                 \
    do                                                                         \
    {                                                                          \
        const auto &_guard_a = (a);                                            \
        const auto &_guard_b = (b);                                            \
        if (!(_guard_a > _guard_b))                                            \
        {                                                                      \
            GUARD_CURRENT_LOCATION(loc);                                       \
            std::ostringstream _guard_os;                                      \
            _guard_os << guard_location_part(loc) << "\tcond:"                 \
                      << GUARD_STRINGIFY(a) " > " GUARD_STRINGIFY(b) << "\n"   \
                      << "\tleft: " << _guard_a << "\n"                        \
                      << "\tright: " << _guard_b << "\n";                      \
            GUARD_CHECK_ENV_APPEND(_guard_os.str());                           \
            GUARD_CHECK_ENV_RAISE_IMPL();                                      \
        }                                                                      \
    } while (0)

#endif // GUARD_CHECK_H
