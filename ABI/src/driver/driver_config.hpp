#pragma once

#include <Windows.h>
#include <winioctl.h>
#include <cstdint>

#define BP_SEED 0x2F9C6B1F9

namespace bp::config
{
    constexpr uint32_t hash(uint32_t x)
    {
        x ^= x >> 16;
        x *= 0x7feb352d;
        x ^= x >> 15;
        x *= 0x846ca68b;
        x ^= x >> 16;
        return x;
    }

    constexpr uint32_t gen(uint32_t index)
    {
        return hash(static_cast<uint32_t>(BP_SEED) + index);
    }

    constexpr wchar_t to_hex_char(uint32_t value)
    {
        return (value < 10) ? (L'0' + value) : (L'A' + value - 10);
    }

    template<size_t N>
    struct device_name
    {
        wchar_t data[N];

        constexpr device_name(const wchar_t* prefix, uint32_t seed_val) : data{}
        {
            size_t i = 0;
            while (prefix[i] && i < N - 9)
            {
                data[i] = prefix[i];
                i++;
            }

            uint32_t hash_val = hash(seed_val);
            data[i++] = to_hex_char((hash_val >> 28) & 0xF);
            data[i++] = to_hex_char((hash_val >> 24) & 0xF);
            data[i++] = to_hex_char((hash_val >> 20) & 0xF);
            data[i++] = to_hex_char((hash_val >> 16) & 0xF);
            data[i++] = to_hex_char((hash_val >> 12) & 0xF);
            data[i++] = to_hex_char((hash_val >> 8) & 0xF);
            data[i++] = to_hex_char((hash_val >> 4) & 0xF);
            data[i++] = to_hex_char(hash_val & 0xF);
            data[i] = L'\0';
        }
    };

    constexpr auto device_name_str = device_name<64>(L"\\Device\\", static_cast<uint32_t>(BP_SEED));
    constexpr auto device_link_str = device_name<64>(L"\\DosDevices\\", static_cast<uint32_t>(BP_SEED));
    constexpr auto device_path_str = device_name<64>(L"\\\\.\\", static_cast<uint32_t>(BP_SEED));

    inline constexpr uint32_t CODE_RW = CTL_CODE(FILE_DEVICE_UNKNOWN, (gen(1) & 0xFFF) | 0x9000, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    inline constexpr uint32_t CODE_BA = CTL_CODE(FILE_DEVICE_UNKNOWN, (gen(2) & 0xFFF) | 0x9000, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    inline constexpr uint32_t CODE_GET_DIRBASE = CTL_CODE(FILE_DEVICE_UNKNOWN, (gen(3) & 0xFFF) | 0x9000, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    inline constexpr uint32_t CODE_MOUSE_MOVE = CTL_CODE(FILE_DEVICE_UNKNOWN, (gen(4) & 0xFFF) | 0x9000, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    inline constexpr uint32_t CODE_SECURITY = gen(10);

    inline constexpr const wchar_t* DEVICE_NAME = device_name_str.data;
    inline constexpr const wchar_t* DEVICE_LINK = device_link_str.data;
    inline constexpr const wchar_t* DEVICE_PATH = device_path_str.data;
}
