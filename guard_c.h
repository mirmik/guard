#ifndef GUARD_C_H
#define GUARD_C_H

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*guard_c_test_func)(void);

typedef struct guard_c_state
{
    FILE* out;
    const char* filter;
    int verbose;
    int tests_total;
    int tests_passed;
    int tests_failed;
    int asserts_total;
    int asserts_failed;
    const char* current_test;
    const char* current_file;
    int current_line;
    int current_failed;
    int current_asserts;
    int current_failed_asserts;
} guard_c_state;

static guard_c_state* guard_c_get_state(void)
{
    static guard_c_state state;
    return &state;
}

static int guard_c_streq(const char* lhs, const char* rhs)
{
    if (lhs == NULL || rhs == NULL)
    {
        return lhs == rhs;
    }
    return strcmp(lhs, rhs) == 0;
}

static int guard_c_contains(const char* text, const char* needle)
{
    if (needle == NULL || needle[0] == '\0')
    {
        return 1;
    }
    if (text == NULL)
    {
        return 0;
    }
    return strstr(text, needle) != NULL;
}

static void guard_c_begin(const char* filter, FILE* out)
{
    guard_c_state* state = guard_c_get_state();
    memset(state, 0, sizeof(*state));
    state->filter = filter;
    state->out = out != NULL ? out : stdout;
}

static void guard_c_begin_args(int argc, char** argv, FILE* out)
{
    const char* filter = NULL;
    int verbose = 0;
    int i;

    for (i = 1; i < argc; ++i)
    {
        if (strncmp(argv[i], "--test-case=", 12) == 0)
        {
            filter = argv[i] + 12;
        }
        else if (strcmp(argv[i], "--test-case") == 0 && i + 1 < argc)
        {
            ++i;
            filter = argv[i];
        }
        else if (strcmp(argv[i], "--verbose") == 0)
        {
            verbose = 1;
        }
    }

    guard_c_begin(filter, out);
    guard_c_get_state()->verbose = verbose;
}

static void guard_c_record_assert(
    int ok,
    const char* expr,
    const char* file,
    int line,
    const char* func,
    const char* detail)
{
    guard_c_state* state = guard_c_get_state();
    FILE* out = state->out != NULL ? state->out : stdout;
    ++state->asserts_total;
    ++state->current_asserts;

    if (ok)
    {
        return;
    }

    ++state->asserts_failed;
    ++state->current_failed_asserts;
    state->current_failed = 1;

    fprintf(out, "%s:%d: check failed in %s\n", file, line, func);
    fprintf(out, "  test: %s\n", state->current_test ? state->current_test : "(unknown)");
    fprintf(out, "  expr: %s\n", expr);
    if (detail != NULL && detail[0] != '\0')
    {
        fprintf(out, "  %s\n", detail);
    }
}

static void guard_c_run(const char* name, guard_c_test_func func, const char* file, int line)
{
    guard_c_state* state = guard_c_get_state();
    FILE* out = state->out != NULL ? state->out : stdout;
    int rc;

    if (!guard_c_contains(name, state->filter))
    {
        return;
    }

    ++state->tests_total;
    state->current_test = name;
    state->current_file = file;
    state->current_line = line;
    state->current_failed = 0;
    state->current_asserts = 0;
    state->current_failed_asserts = 0;

    if (state->verbose)
    {
        fprintf(out, "Running test: \"%s\" (%s:%d)\n", name, file, line);
    }

    rc = func();
    if (rc != 0)
    {
        state->current_failed = 1;
    }

    if (state->current_failed)
    {
        ++state->tests_failed;
        fprintf(out,
                "[FAIL] %s (%s:%d, asserts %d, failed %d)\n",
                name,
                file,
                line,
                state->current_asserts,
                state->current_failed_asserts);
    }
    else
    {
        ++state->tests_passed;
        if (state->verbose)
        {
            fprintf(out, "[ OK ] %s (%d asserts)\n", name, state->current_asserts);
        }
    }

    state->current_test = NULL;
    state->current_file = NULL;
    state->current_line = 0;
}

static int guard_c_end(void)
{
    guard_c_state* state = guard_c_get_state();
    FILE* out = state->out != NULL ? state->out : stdout;

    fprintf(out, "=======================\n");
    fprintf(out, "C guard summary:\n");
    fprintf(out, "Tests run : %d\n", state->tests_total);
    fprintf(out, "Passed    : %d\n", state->tests_passed);
    fprintf(out, "Failed    : %d\n", state->tests_failed);
    fprintf(out, "Asserts   : %d", state->asserts_total);
    if (state->asserts_failed > 0)
    {
        fprintf(out, " (failed %d)", state->asserts_failed);
    }
    fprintf(out, "\n");
    fprintf(out, "=======================\n");

    return state->tests_failed == 0 ? 0 : 1;
}

#define GUARD_C_BEGIN() guard_c_begin(NULL, stdout)
#define GUARD_C_BEGIN_FILTER(filter_) guard_c_begin((filter_), stdout)
#define GUARD_C_BEGIN_ARGS(argc_, argv_) guard_c_begin_args((argc_), (argv_), stdout)
#define GUARD_C_RUN(test_) guard_c_run(#test_, (test_), __FILE__, __LINE__)
#define GUARD_C_END() guard_c_end()
#define GUARD_C_TEST(name_) static int name_(void)

#define GUARD_C_CHECK(expr_)                                                   \
    do                                                                         \
    {                                                                          \
        const int guard_c_ok_ = !!(expr_);                                      \
        guard_c_record_assert(guard_c_ok_, #expr_, __FILE__, __LINE__, __func__, NULL); \
    } while (0)

#define GUARD_C_REQUIRE(expr_)                                                 \
    do                                                                         \
    {                                                                          \
        const int guard_c_ok_ = !!(expr_);                                      \
        guard_c_record_assert(guard_c_ok_, #expr_, __FILE__, __LINE__, __func__, NULL); \
        if (!guard_c_ok_)                                                       \
        {                                                                      \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define GUARD_C_CHECK_FALSE(expr_) GUARD_C_CHECK(!(expr_))
#define GUARD_C_REQUIRE_FALSE(expr_) GUARD_C_REQUIRE(!(expr_))

#define GUARD_C_FAIL(message_)                                                  \
    do                                                                         \
    {                                                                          \
        guard_c_record_assert(0, "GUARD_C_FAIL", __FILE__, __LINE__, __func__, (message_)); \
        return 1;                                                              \
    } while (0)

#define GUARD_C_CHECK_EQ_INT(expected_, actual_)                                \
    do                                                                         \
    {                                                                          \
        const int guard_c_expected_ = (expected_);                              \
        const int guard_c_actual_ = (actual_);                                  \
        char guard_c_detail_[128];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %d, actual: %d", guard_c_expected_, guard_c_actual_); \
        guard_c_record_assert(guard_c_expected_ == guard_c_actual_, #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
    } while (0)

#define GUARD_C_REQUIRE_EQ_INT(expected_, actual_)                              \
    do                                                                         \
    {                                                                          \
        const int guard_c_expected_ = (expected_);                              \
        const int guard_c_actual_ = (actual_);                                  \
        char guard_c_detail_[128];                                             \
        const int guard_c_ok_ = guard_c_expected_ == guard_c_actual_;           \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %d, actual: %d", guard_c_expected_, guard_c_actual_); \
        guard_c_record_assert(guard_c_ok_, #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
        if (!guard_c_ok_)                                                       \
        {                                                                      \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define GUARD_C_CHECK_EQ_UINT(expected_, actual_)                               \
    do                                                                         \
    {                                                                          \
        const unsigned int guard_c_expected_ = (expected_);                     \
        const unsigned int guard_c_actual_ = (actual_);                         \
        char guard_c_detail_[128];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %u, actual: %u", guard_c_expected_, guard_c_actual_); \
        guard_c_record_assert(guard_c_expected_ == guard_c_actual_, #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
    } while (0)

#define GUARD_C_CHECK_EQ_LONG(expected_, actual_)                               \
    do                                                                         \
    {                                                                          \
        const long guard_c_expected_ = (expected_);                             \
        const long guard_c_actual_ = (actual_);                                 \
        char guard_c_detail_[160];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %ld, actual: %ld", guard_c_expected_, guard_c_actual_); \
        guard_c_record_assert(guard_c_expected_ == guard_c_actual_, #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
    } while (0)

#define GUARD_C_CHECK_EQ_SIZE(expected_, actual_)                               \
    do                                                                         \
    {                                                                          \
        const size_t guard_c_expected_ = (expected_);                           \
        const size_t guard_c_actual_ = (actual_);                               \
        char guard_c_detail_[160];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %lu, actual: %lu", (unsigned long)guard_c_expected_, (unsigned long)guard_c_actual_); \
        guard_c_record_assert(guard_c_expected_ == guard_c_actual_, #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
    } while (0)

#define GUARD_C_CHECK_NEAR_DOUBLE(expected_, actual_, epsilon_)                 \
    do                                                                         \
    {                                                                          \
        const double guard_c_expected_ = (expected_);                           \
        const double guard_c_actual_ = (actual_);                               \
        const double guard_c_epsilon_ = (epsilon_);                             \
        const double guard_c_diff_ = fabs(guard_c_expected_ - guard_c_actual_); \
        char guard_c_detail_[192];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %.17g, actual: %.17g, diff: %.17g, epsilon: %.17g", guard_c_expected_, guard_c_actual_, guard_c_diff_, guard_c_epsilon_); \
        guard_c_record_assert(guard_c_diff_ <= guard_c_epsilon_, #expected_ " ~= " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
    } while (0)

#define GUARD_C_REQUIRE_NEAR_DOUBLE(expected_, actual_, epsilon_)               \
    do                                                                         \
    {                                                                          \
        const double guard_c_expected_ = (expected_);                           \
        const double guard_c_actual_ = (actual_);                               \
        const double guard_c_epsilon_ = (epsilon_);                             \
        const double guard_c_diff_ = fabs(guard_c_expected_ - guard_c_actual_); \
        const int guard_c_ok_ = guard_c_diff_ <= guard_c_epsilon_;              \
        char guard_c_detail_[192];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %.17g, actual: %.17g, diff: %.17g, epsilon: %.17g", guard_c_expected_, guard_c_actual_, guard_c_diff_, guard_c_epsilon_); \
        guard_c_record_assert(guard_c_ok_, #expected_ " ~= " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
        if (!guard_c_ok_)                                                       \
        {                                                                      \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define GUARD_C_CHECK_STREQ(expected_, actual_)                                 \
    do                                                                         \
    {                                                                          \
        const char* guard_c_expected_ = (expected_);                            \
        const char* guard_c_actual_ = (actual_);                                \
        char guard_c_detail_[256];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: \"%s\", actual: \"%s\"", guard_c_expected_ ? guard_c_expected_ : "(null)", guard_c_actual_ ? guard_c_actual_ : "(null)"); \
        guard_c_record_assert(guard_c_streq(guard_c_expected_, guard_c_actual_), #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
    } while (0)

#define GUARD_C_REQUIRE_STREQ(expected_, actual_)                               \
    do                                                                         \
    {                                                                          \
        const char* guard_c_expected_ = (expected_);                            \
        const char* guard_c_actual_ = (actual_);                                \
        const int guard_c_ok_ = guard_c_streq(guard_c_expected_, guard_c_actual_); \
        char guard_c_detail_[256];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: \"%s\", actual: \"%s\"", guard_c_expected_ ? guard_c_expected_ : "(null)", guard_c_actual_ ? guard_c_actual_ : "(null)"); \
        guard_c_record_assert(guard_c_ok_, #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
        if (!guard_c_ok_)                                                       \
        {                                                                      \
            return 1;                                                          \
        }                                                                      \
    } while (0)

#define GUARD_C_CHECK_PTR_EQ(expected_, actual_)                                \
    do                                                                         \
    {                                                                          \
        const void* guard_c_expected_ = (const void*)(expected_);               \
        const void* guard_c_actual_ = (const void*)(actual_);                   \
        char guard_c_detail_[160];                                             \
        snprintf(guard_c_detail_, sizeof(guard_c_detail_), "expected: %p, actual: %p", guard_c_expected_, guard_c_actual_); \
        guard_c_record_assert(guard_c_expected_ == guard_c_actual_, #expected_ " == " #actual_, __FILE__, __LINE__, __func__, guard_c_detail_); \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* GUARD_C_H */
