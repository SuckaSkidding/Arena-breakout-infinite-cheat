#pragma once

#include <Windows.h>
#include <cstdint>
#include "ioctl.hpp"

namespace bp::core
{
    class driver
    {
    public:
        static bool connect() noexcept;
        static void disconnect() noexcept;

        static int32_t find_process(const wchar_t* process_name) noexcept;
        static bool attach(int32_t pid) noexcept;

        static bool get_cr3() noexcept;
        static bool get_module_base(uintptr_t& out_base) noexcept;
        static uintptr_t get_base_address() noexcept { return module_base; }

        static bool read_physical(uintptr_t address, void* __restrict buffer, size_t size) noexcept;

        template<typename T>
        static bool read(uintptr_t address, T& out_value) noexcept
        {
            return read_physical(address, &out_value, sizeof(T));
        }

        static HANDLE get_handle() noexcept { return handle; }
        static int32_t get_process_id() noexcept { return process_id; }

    private:
        inline static HANDLE handle = INVALID_HANDLE_VALUE;
        inline static int32_t process_id = 0;
        inline static uintptr_t directory_base = 0;
        inline static uintptr_t module_base = 0;
    };
}
