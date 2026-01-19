#pragma once
#include "../driver/manager.hpp"
#include <cstdint>
#include <string_view>
#include <tlhelp32.h> // Pour la recherche de processus

class MemoryManager
{
public:
    uintptr_t BaseAddress = 0;

    // Initialise le gestionnaire de mémoire et trouve l'adresse de base
    bool initialize() noexcept
    {
        bp::core::driver::connect();

      /*  if (!bp::memory::manager::initialize())
        {
            return false;
        }*/

        // Trouve l'adresse de base du processus cible
        
        BaseAddress = find_process_base(L"UAGame.exe");
        if (BaseAddress == 0) {
            std::cout << "[+] " << "BaseAddress Not found" << std::endl;

        }

        return BaseAddress != 0;
    }

    // Arrête le gestionnaire de mémoire
    void shutdown() const noexcept
    {
        bp::memory::manager::shutdown();
    }

    // Trouve l'adresse de base d'un processus par son nom
    uintptr_t find_process_base(const wchar_t* process_name) const
    {
        uint32_t pid = bp::core::driver::find_process(process_name);
        if (pid == 0)
        {
            std::cout << "[+] " << "PID Not found" << std::endl;
            std::cout << "[+] " << "Pid :" << pid << std::endl;

            return 0; // Processus non trouvé
        }
        bp::core::driver::attach(pid);
        uintptr_t BaseAddresss = 0;
        bp::core::driver::get_module_base(BaseAddresss);
      //  std::cout << (BaseAddresss);
        if (bp::core::driver::get_base_address() == 0) {
            if (BaseAddresss == 0) {
                std::cout << "[+] " << "ba not found :" << pid << std::endl;

            }
            else {
                return BaseAddresss;
            }
        }
        else {
            return bp::core::driver::get_base_address();

        }
    }

    // Lit une valeur depuis une adresse mémoire
    template<typename T>
    T Read(uintptr_t address) const
    {
        return bp::memory::manager::read<T>(address);
    }
    bool ReadString(uintptr_t address, char* buffer, size_t size) const noexcept
    {
        if (!buffer || size == 0)
        {
            return false;
        }

        for (size_t i = 0; i < size; ++i)
        {
            // Lit chaque caractère un par un
            buffer[i] = Read<char>(address + i);
        }
        // Note : La fonction ne place pas de terminateur nul.
        // C'est à l'appelant de le faire s'il en a besoin.
        return true;
    }
    // Lit une valeur de manière sécurisée (vérifie l'adresse et retourne un booléen)
    template<typename T>
    bool Read_safe(uintptr_t address, T& value) const noexcept
    {
        return bp::memory::manager::read_safe(address, value);
    }

    bool read_raw(uintptr_t address, void* buffer, size_t size) const noexcept
    {
        return bp::memory::manager::read_raw(address, buffer, size);
    }
    template<typename T>
    bool write(uintptr_t address, const T& value) const noexcept
    {
        return bp::memory::manager::write(address, value);
    }
    // NOUVELLE FONCTION ReadArray
    template<typename T>
    bool ReadArray(uintptr_t address, T* buffer, size_t count) const noexcept
    {
        if (!buffer || count == 0)
        {
            return false;
        }
        // Utilise la fonction de lecture de bloc du driver
        return bp::memory::manager::read_raw(address, buffer, sizeof(T) * count);
    }

};

// Instance globale pour un accès facile (mem.read, mem.write, etc.)
inline MemoryManager mem;