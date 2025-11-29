// guard.h (или guard/test.h)
#pragma once

#include "check.h"
#include "env.h"
#include "util.h"
#include <algorithm>
#include <map>

#include <chrono>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace guard::detail
{
    enum class Color
    {
        Default,
        Green,
        Red,
        Yellow
    };

    inline const char *color_code(Color c)
    {
#ifdef GUARD_TEST_ENABLE_COLORS
        switch (c)
        {
        case Color::Green:
            return "\033[32m";
        case Color::Red:
            return "\033[31m";
        case Color::Yellow:
            return "\033[33m";
        case Color::Default:
        default:
            return "\033[0m";
        }
#else
        (void)c;
        return "";
#endif
    }

    struct ColorScope
    {
        std::ostream &os;

        explicit ColorScope(std::ostream &os_, Color c) : os(os_)
        {
            os << color_code(c);
        }

        ~ColorScope()
        {
            os << color_code(Color::Default);
        }
    };
} // namespace guard::detail
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
    struct ModuleStats
    {
        std::string file;
        int tests_total = 0;
        int tests_passed = 0;
        int tests_failed = 0;
        unsigned long long asserts_total = 0;
        unsigned long long asserts_failed = 0;
    };

    // test_filter == nullptr -> запускать все тесты
    inline int run_all(const char *test_filter, std::ostream &os = std::cout)
    {
        RunnerStats stats;
        std::map<std::string, ModuleStats> modules;

        using guard::detail::Color;
        using guard::detail::ColorScope;

        struct TestSummary
        {
            const TestCase *tc;
            std::string error;
        };

        std::vector<TestSummary> failures;

        // Копируем и сортируем тесты по файлу, строке и имени
        auto tests = registry();
        std::sort(tests.begin(), tests.end(), [](const TestCase &lhs, const TestCase &rhs) {
            const std::string lhs_file(lhs.file ? lhs.file : "");
            const std::string rhs_file(rhs.file ? rhs.file : "");
            if (lhs_file < rhs_file)
                return true;
            if (rhs_file < lhs_file)
                return false;
            if (lhs.line != rhs.line)
                return lhs.line < rhs.line;
            const std::string lhs_name(lhs.name ? lhs.name : "");
            const std::string rhs_name(rhs.name ? rhs.name : "");
            return lhs_name < rhs_name;
        });

        for (const auto &tc : tests)
        {
            if (test_filter &&
                std::string(tc.name).find(test_filter) == std::string::npos)
                continue;

            ++stats.total;

            auto &mod = modules[tc.file];
            if (mod.file.empty())
                mod.file = tc.file;
            ++mod.tests_total;

            guard_check_error_msg.clear();

            guard_check_env_t &env = guard_check_env();
            const auto asserts_before_total = env.assert_total;
            const auto asserts_before_failed = env.assert_failed;

            bool test_passed = true;
            std::string test_error;

            GUARD_CHECK_ENV_START()
            {
                try
                {
                    tc.func();

                    if (guard_check_error_msg.empty())
                    {
                        ++stats.passed;
                        ++mod.tests_passed;
                        test_passed = true;
                    }
                    else
                    {
                        ++stats.failed;
                        ++mod.tests_failed;
                        test_passed = false;
                        test_error = guard_check_error_msg;
                    }
                }
                catch (const guard_check_exception &)
                {
                    // REQUIRE/FAIL бросают guard_check_exception — пробрасываем наружу
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
                ++mod.tests_failed;
                test_passed = false;
                if (!guard_check_error_msg.empty())
                    test_error = guard_check_error_msg;
            }

            const auto asserts_after_total = env.assert_total;
            const auto asserts_after_failed = env.assert_failed;
            mod.asserts_total += (asserts_after_total - asserts_before_total);
            mod.asserts_failed += (asserts_after_failed - asserts_before_failed);

            if (!test_passed)
            {
                failures.push_back(TestSummary{&tc, std::move(test_error)});
            }
        }

        os << "=======================\n";
        os << "Per-module summary:\n";
        for (const auto &entry : modules)
        {
            const auto &mod = entry.second;

            Color mod_color =
                (mod.tests_failed > 0 || mod.asserts_failed > 0)
                    ? Color::Red
                    : Color::Green;

            {
                ColorScope scope(os, mod_color);
                os << mod.file << ":\n";
            }

            os << "  Tests   : " << mod.tests_total
               << " (passed " << mod.tests_passed
               << ", failed " << mod.tests_failed << ")\n";
            os << "  Asserts : " << mod.asserts_total;
            if (mod.asserts_failed > 0)
                os << " (failed " << mod.asserts_failed << ")";
            os << "\n";
        }

        guard_check_env_t &env = guard_check_env();
        os << "=======================\n";
        {
            ColorScope summary_scope(os, stats.failed == 0 ? Color::Green : Color::Red);

            os << "Overall summary:\n";

            os << "Tests run : " << stats.total << "\n";

            os << "Passed    : ";
            {
                ColorScope passed_scope(os, stats.passed > 0 ? Color::Green : Color::Default);
                os << stats.passed;
            }
            os << "\n";

            os << "Failed    : ";
            {
                ColorScope failed_scope(os, stats.failed > 0 ? Color::Red : Color::Default);
                os << stats.failed;
            }
            os << "\n";
        }
        os << "Asserts   : " << env.assert_total
           << " (failed " << env.assert_failed << ")\n";

        os << "=======================\n";
        {
            ColorScope scope(os, failures.empty() ? Color::Green : Color::Red);
            os << "Failures detail:\n";
        }
        if (failures.empty())
        {
            os << "No test failures.\n";
        }
        else
        {
            for (const auto &f : failures)
            {
                {
                    ColorScope scope(os, Color::Red);
                    os << f.tc->file << ":" << f.tc->line
                       << " in test \"" << f.tc->name << "\"\n";
                }
                if (!f.error.empty())
                    os << f.error << "\n";
                os << "-----------------------\n";
            }
        }

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
        GUARD_CHECK_ENV_COUNT_ASSERT(false);                                   \
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
        else                                                                   \
        {                                                                      \
            GUARD_CHECK_ENV_COUNT_ASSERT(true);                                \
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
        else                                                                   \
        {                                                                      \
            GUARD_CHECK_ENV_COUNT_ASSERT(true);                                \
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
        GUARD_CHECK_ENV_COUNT_ASSERT(true);                                    \
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
        else                                                                   \
        {                                                                      \
            GUARD_CHECK_ENV_COUNT_ASSERT(true);                                \
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
