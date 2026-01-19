#pragma once

#include <Windows.h>
#include <cstdint>
#include "driver_config.hpp"

namespace bp::core
{
    struct transfer_data
    {
        int32_t key;
        int32_t target;
        uint64_t src;
        uint64_t dst;
        uint64_t len;
        bool mode;
    };

    struct module_info
    {
        int32_t key;
        int32_t target;
        uint64_t* result;
    };

    struct context_data
    {
        uint32_t target;
        uintptr_t value;
    };

    struct mouse_move_data
    {
        int32_t key;
        long x;
        long y;
        unsigned short button_flags;
    };
}
