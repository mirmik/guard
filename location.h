#pragma once

struct guard_location
{
    int line;
    const char *file;
    const char *func;
};

#define GUARD_CURRENT_LOCATION(name)                                           \
    struct guard_location name = {__LINE__, __FILE__, __func__};

#define GUARD_CURRENT_LOCATION_INITARGS __LINE__, __FILE__, __func__