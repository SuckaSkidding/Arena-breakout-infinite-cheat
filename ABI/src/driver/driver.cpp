#include "driver.hpp"
#include "ioctl.hpp"
#include "driver_config.hpp"
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <iomanip>
#include <cstring>

using namespace bp::core;

namespace bp::core
{
    using namespace bp::config;

    int32_t driver::find_process(const wchar_t* process_name) noexcept
    {
        PROCESSENTRY32W entry = { sizeof(PROCESSENTRY32W) };
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        if (Process32FirstW(snapshot, &entry))
        {
            do
            {
                if (_wcsicmp(entry.szExeFile, process_name) == 0)
                {
                    CloseHandle(snapshot);
                    return entry.th32ProcessID;
                }
            } while (Process32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return 0;
    }

    bool driver::attach(int32_t pid) noexcept
    {
        if (pid == 0)
            return false;
        process_id = pid;
        return true;
    }

    bool driver::connect() noexcept
    {
        handle = CreateFileW(
            config::DEVICE_PATH,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        return handle != INVALID_HANDLE_VALUE;
    }

    void driver::disconnect() noexcept
    {
        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
    }

    bool driver::get_cr3() noexcept
    {
        if (handle == INVALID_HANDLE_VALUE || process_id == 0)
            return false;

        context_data args = { 0 };
        args.target = process_id;
        args.value = reinterpret_cast<uintptr_t>(&directory_base);

        DWORD bytes_returned = 0;
        bool result = DeviceIoControl(
            handle,
            config::CODE_GET_DIRBASE,
            &args,
            sizeof(args),
            nullptr,
            0,
            &bytes_returned,
            nullptr
        );

        return result && directory_base != 0;
    }

    bool driver::get_module_base(uintptr_t& out_base) noexcept
    {
        if (handle == INVALID_HANDLE_VALUE || process_id == 0)
        {
            //std::cout << "INVALID HANDLE";
            out_base = 0;
            return false;
        }

        out_base = 0;
        module_info args = { 0 };
        args.key = config::CODE_SECURITY;
        args.target = process_id;
        args.result = &out_base;

        DWORD bytes_returned = 0;
        bool result = DeviceIoControl(
            handle,
            config::CODE_BA,
            &args,
            sizeof(args),
            nullptr,
            0,
            &bytes_returned,
            nullptr
        );

        if (result && out_base != 0)
        {
            module_base = out_base;
            return true;
        }

        return false;
    }

    bool driver::read_physical(uintptr_t address, void* __restrict buffer, size_t size) noexcept
    {
        if (handle == INVALID_HANDLE_VALUE || process_id == 0 || !buffer || size == 0)
            return false;

        transfer_data args = { 0 };
        args.key = config::CODE_SECURITY;
        args.target = process_id;
        args.src = address;
        args.dst = reinterpret_cast<uint64_t>(buffer);
        args.len = size;
        args.mode = false;

        DWORD bytes_returned = 0;
        return DeviceIoControl(
            handle,
            config::CODE_RW,
            &args,
            sizeof(args),
            nullptr,
            0,
            &bytes_returned,
            nullptr
        );
    }
   

}
