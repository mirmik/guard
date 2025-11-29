// guard/check/env.h
#pragma once

#include <exception>
#include <string>

// Вся среда проверки в одной структуре
struct guard_check_env_t
{
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
#define guard_check_error_msg (guard_check_env().error_msg)

// Начало "окружения" проверки: очищаем строку и запускаем try-блок
#define GUARD_CHECK_ENV_START()                                                \
    if (guard_check_error_msg.clear(), true)                                   \
        try

// Ветка обработки ошибки (после выброса guard_check_exception)
#define GUARD_CHECK_ENV_ERROR_HANDLER() catch (const guard_check_exception &)

// Установка сообщения об ошибке (перезаписывает предыдущий текст)
inline void GUARD_CHECK_ENV_RAISE_SET(const std::string &msg)
{
    guard_check_error_msg = msg;
}

// Переход назад в точку GUARD_CHECK_ENV_START()
class guard_check_exception : public std::exception
{
public:
    const char *what() const noexcept override
    {
        return "guard test failure";
    }
};
inline void GUARD_CHECK_ENV_RAISE_IMPL()
{
    throw guard_check_exception{};
}

// Добавление сообщения об ошибке (для "мягких" CHECK)
inline void GUARD_CHECK_ENV_APPEND(const std::string &msg)
{
    if (!guard_check_error_msg.empty())
        guard_check_error_msg.push_back('\n');
    guard_check_error_msg += msg;
}
