#include "manager.hpp"

namespace bp::memory
{
    bool manager::initialize() noexcept
    {
        return core::driver::get_handle() != INVALID_HANDLE_VALUE;
    }

    void manager::shutdown() noexcept
    {
    }
    bool bp::memory::manager::read_raw(uintptr_t address, void* buffer, size_t size) noexcept
    {
        // Appelle la fonction de bas niveau du driver
        return core::driver::read_physical(address, buffer, size);
    }
}
