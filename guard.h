// guard.h (или guard/test.h)
#pragma once

#include "check.h"
#include "env.h"
#include "util.h"

#include <chrono>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace guard::test
{
    using TestFunc = void (*)();

    struct TestCase
    {
        const char *name;
        const char *file;
        int line;
        TestFunc func;
    };

    inline std::vector<TestCase> &registry()
    {
        static std::vector<TestCase> instance;
        return instance;
    }

    struct Registrar
    {
        Registrar(const char *name, const char *file, int line, TestFunc func)
        {
            registry().push_back(TestCase{name, file, line, func});
        }
    };

    struct RunnerStats
    {
        int total = 0;
        int passed = 0;
        int failed = 0;
    };

    // test_filter == nullptr -> запускать все тесты
    inline int run_all(const char *test_filter, std::ostream &os = std::cout)
    {
        RunnerStats stats;

        for (const auto &tc : registry())
        {
            if (test_filter &&
                std::string(tc.name).find(test_filter) == std::string::npos)
                continue;

            ++stats.total;
            os << "[ RUN      ] " << tc.name << "  (" << tc.file << ":"
               << tc.line << ")\n";

            guard_check_error_msg.clear();

            GUARD_CHECK_ENV_START()
            {
                try
                {
                    tc.func();

                    if (guard_check_error_msg.empty())
                    {
                        ++stats.passed;
                        os << "[       OK ] " << tc.name << "\n";
                    }
                    else
                    {
                        ++stats.failed;
                        os << "[  FAILED  ] " << tc.name << "\n";
                        os << guard_check_error_msg << "\n";
                    }
                }
                catch (const guard_check_exception &)
                {
                    // Это «нормальный» путь завершения теста через
                    // REQUIRE/FAIL. Сообщения уже лежат в
                    // guard_check_error_msg.
                    throw;
                }
                catch (const std::exception &ex)
                {
                    std::string msg =
                        std::string("Unexpected std::exception in test \"") +
                        tc.name + "\": " + ex.what();
                    GUARD_CHECK_ENV_RAISE_SET(msg);
                    GUARD_CHECK_ENV_RAISE_IMPL();
                }
                catch (...)
                {
                    std::string msg =
                        std::string("Unexpected non-std exception in test \"") +
                        tc.name + "\"";
                    GUARD_CHECK_ENV_RAISE_SET(msg);
                    GUARD_CHECK_ENV_RAISE_IMPL();
                }
            }
            GUARD_CHECK_ENV_ERROR_HANDLER()
            {
                ++stats.failed;
                os << "[  FAILED  ] " << tc.name << "\n";
                if (!guard_check_error_msg.empty())
                    os << guard_check_error_msg << "\n";
            }
        }

        os << "=======================\n";
        os << "Tests run : " << stats.total << "\n";
        os << "Passed    : " << stats.passed << "\n";
        os << "Failed    : " << stats.failed << "\n";

        return stats.failed ? 1 : 0;
    }

    inline int run_all(std::ostream &os = std::cout)
    {
        return run_all(nullptr, os);
    }
} // namespace guard::test

// ---------- внутренняя склейка имён ----------
#define GUARD_TEST_CONCAT_IMPL(a, b) a##b
#define GUARD_TEST_CONCAT(a, b) GUARD_TEST_CONCAT_IMPL(a, b)

// ---------- PUBLIC API: TEST_CASE ----------
//
// TEST_CASE("name") {
//     CHECK(...);
// }
#define TEST_CASE(name)                                                        \
    static void GUARD_TEST_CONCAT(guard_test_func_, __LINE__)();               \
    static ::guard::test::Registrar GUARD_TEST_CONCAT(guard_test_reg_,         \
                                                      __LINE__)(               \
        name,                                                                  \
        __FILE__,                                                              \
        __LINE__,                                                              \
        &GUARD_TEST_CONCAT(guard_test_func_, __LINE__));                       \
    static void GUARD_TEST_CONCAT(guard_test_func_, __LINE__)()

// ---------- Алисы CHECK* / REQUIRE* на GUARD_* ----------

#ifndef GUARD_TEST_NO_CHECK_ALIASES
#define CHECK(...) GUARD_CHECK(__VA_ARGS__)
#define CHECK_FALSE(expr) GUARD_CHECK_FALSE(expr)
#define CHECK_EQ(a, b) GUARD_CHECK_EQ(a, b)
#define CHECK_NEQ(a, b) GUARD_CHECK_NEQ(a, b)
#define CHECK_LT(a, b) GUARD_CHECK_LT(a, b)
#define CHECK_GT(a, b) GUARD_CHECK_GT(a, b)

#define REQUIRE(...) GUARD_REQUIRE(__VA_ARGS__)
#define REQUIRE_FALSE(expr) GUARD_REQUIRE_FALSE(expr)
#define REQUIRE_EQ(a, b) GUARD_REQUIRE_EQ(a, b)
#define REQUIRE_NEQ(a, b) GUARD_REQUIRE_NEQ(a, b)
#define REQUIRE_LT(a, b) GUARD_REQUIRE_LT(a, b)
#define REQUIRE_GT(a, b) GUARD_REQUIRE_GT(a, b)
#define FAIL(msg) GUARD_TEST_FAIL_MSG(msg)
#endif

// ---------- общий helper для "сделать фейл" ----------
#define GUARD_TEST_FAIL_MSG(msg_)                                              \
    do                                                                         \
    {                                                                          \
        std::string _guard_msg = std::string("Test assertion failed at ") +    \
                                 __FILE__ + ":" + std::to_string(__LINE__) +   \
                                 ": " + (msg_);                                \
        GUARD_CHECK_ENV_APPEND(_guard_msg);                                    \
        GUARD_CHECK_ENV_RAISE_IMPL();                                          \
    } while (0)

// ---------- проверки исключений ----------

// ожидаем любое исключение (фатальный, ближе к REQUIRE_THROWS)
#define CHECK_THROWS(code)                                                     \
    do                                                                         \
    {                                                                          \
        bool _guard_thrown = false;                                            \
        try                                                                    \
        {                                                                      \
            code;                                                              \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            _guard_thrown = true;                                              \
        }                                                                      \
        if (!_guard_thrown)                                                    \
        {                                                                      \
            GUARD_TEST_FAIL_MSG(std::string("Expected exception from: ") +     \
                                #code);                                        \
        }                                                                      \
    } while (0)

// ожидаем, что code кинет исключение типа ExType (фатальный)
#define CHECK_THROWS_AS(code, ExType)                                          \
    do                                                                         \
    {                                                                          \
        bool _guard_ok = false;                                                \
        try                                                                    \
        {                                                                      \
            code;                                                              \
        }                                                                      \
        catch (const ExType &)                                                 \
        {                                                                      \
            _guard_ok = true;                                                  \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
        }                                                                      \
        if (!_guard_ok)                                                        \
        {                                                                      \
            GUARD_TEST_FAIL_MSG(std::string("Expected exception of type ") +   \
                                #ExType + " from: " #code);                    \
        }                                                                      \
    } while (0)

// ожидаем, что code НИЧЕГО не кидает (фатальный)
#define CHECK_NOTHROW(code)                                                    \
    do                                                                         \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            code;                                                              \
        }                                                                      \
        catch (const std::exception &_ex)                                      \
        {                                                                      \
            GUARD_TEST_FAIL_MSG(                                               \
                std::string("Unexpected std::exception from: ") + #code +      \
                ", what(): " + _ex.what());                                    \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            GUARD_TEST_FAIL_MSG(                                               \
                std::string("Unexpected non-std exception from: ") + #code);   \
        }                                                                      \
    } while (0)

// ---------- "таймаут" по времени выполнения ----------
#define CHECK_TIMEOUT(code, ms)                                                \
    do                                                                         \
    {                                                                          \
        auto _guard_start = std::chrono::steady_clock::now();                  \
        code;                                                                  \
        auto _guard_end = std::chrono::steady_clock::now();                    \
        auto _guard_ms =                                                       \
            std::chrono::duration_cast<std::chrono::milliseconds>(             \
                _guard_end - _guard_start)                                     \
                .count();                                                      \
        if (_guard_ms > (ms))                                                  \
        {                                                                      \
            GUARD_TEST_FAIL_MSG(                                               \
                std::string("Timeout: expression ") + #code + " took " +       \
                std::to_string(_guard_ms) + " ms, limit is " +                 \
                std::to_string(static_cast<long long>(ms)) + " ms");           \
        }                                                                      \
    } while (0)

// ---------- удобный main ----------
// Поддержка фильтрации тестов по имени через аргумент командной строки:
//   --test-case=NameSubstring
//   --test-case NameSubstring
#define GUARD_TEST_MAIN()                                                      \
    int main(int argc, char **argv)                                            \
    {                                                                          \
        const char *test_filter = nullptr;                                     \
        for (int i = 1; i < argc; ++i)                                         \
        {                                                                      \
            const char *arg = argv[i];                                         \
            const char prefix[] = "--test-case=";                              \
            if (std::strncmp(arg, prefix, sizeof(prefix) - 1) == 0)            \
            {                                                                  \
                test_filter = arg + sizeof(prefix) - 1;                        \
            }                                                                  \
            else if (std::strcmp(arg, "--test-case") == 0 && i + 1 < argc)     \
            {                                                                  \
                test_filter = argv[++i];                                       \
            }                                                                  \
        }                                                                      \
        return ::guard::test::run_all(test_filter);                            \
    }
