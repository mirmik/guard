// guard/check/env.h
#pragma once

#include <setjmp.h>
#include <string>

// Вся среда проверки в одной структуре
struct guard_check_env_t
{
    jmp_buf start_point;
    std::string error_msg;
};

// Глобальный (на процесс) экземпляр среды, реализованный через
// статическую локальную переменную внутри inline-функции
inline guard_check_env_t &guard_check_env()
{
    static guard_check_env_t env;
    return env;
}

// Макросы-алиасы, чтобы старый код продолжал работать как раньше
#define guard_check_start_point (guard_check_env().start_point)
#define guard_check_error_msg (guard_check_env().error_msg)

// Начало "окружения" проверки: очищаем строку и делаем setjmp
#define GUARD_CHECK_ENV_START()                                                \
    if (guard_check_error_msg.clear(), setjmp(guard_check_start_point) == 0)

// Ветка обработки ошибки (после longjmp)
#define GUARD_CHECK_ENV_ERROR_HANDLER() else

// Установка сообщения об ошибке (перезаписывает предыдущий текст)
inline void GUARD_CHECK_ENV_RAISE_SET(const std::string &msg)
{
    guard_check_error_msg = msg;
}

// Переход назад в точку GUARD_CHECK_ENV_START()
inline void GUARD_CHECK_ENV_RAISE_IMPL()
{
    longjmp(guard_check_start_point, 1);
}

// Добавление сообщения об ошибке (для "мягких" CHECK)
inline void GUARD_CHECK_ENV_APPEND(const std::string &msg)
{
    if (!guard_check_error_msg.empty())
        guard_check_error_msg.push_back('\n');
    guard_check_error_msg += msg;
}
