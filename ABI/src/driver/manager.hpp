#pragma once

#include <cstdint>
#include "driver.hpp"

namespace bp::memory
{
    class manager
    {
    public:
        static bool initialize() noexcept;
        static void shutdown() noexcept;

        static bool read_raw(uintptr_t address, void* buffer, size_t size) noexcept;


        template<typename T>
        static T read(uintptr_t address)
        {
            T value{};
            read_safe(address, value);
            return value;
        }

        template<typename T>
        static bool read_safe(uintptr_t address, T& value) noexcept
        {
            if (!is_valid_address(address))
                return false;
            return core::driver::read(address, value);
        }

        template<typename T>
        static bool write(uintptr_t address, const T& value) noexcept
        {
            if (!is_valid_address(address))
                return false;
            return core::driver::write(address, value);
        }

        static constexpr bool is_valid_address(uintptr_t address) noexcept
        {
            return address >= MIN_VALID_ADDRESS && address <= MAX_VALID_ADDRESS;
        }

    private:
        static constexpr uintptr_t MIN_VALID_ADDRESS = 0x10000;
        static constexpr uintptr_t MAX_VALID_ADDRESS = 0x7FFFFFFFFFFF;
    };
}
