#pragma once

#include <game/structs.hpp>
#include <game/settings.hpp>
#include <game/math.hpp>

#include <d3d.h>
#include <game/offsets.hpp>
#include "memory.hpp"


namespace sdk {
    bool W2S(FMinimalViewInfo cameraManager, FVector WorldLocation, FVector2D* ScreenPosition, float Zoom = 1.0f) {
        D3DMATRIX tempMatrix = math::Matrix(cameraManager.Rotation);

        FVector vAxisX = FVector(tempMatrix._11, tempMatrix._12, tempMatrix._13);
        FVector vAxisY = FVector(tempMatrix._21, tempMatrix._22, tempMatrix._23);
        FVector vAxisZ = FVector(tempMatrix._31, tempMatrix._32, tempMatrix._33);

        FVector vDelta = WorldLocation - cameraManager.Location;
        FVector vTransformed = FVector(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

        if (vTransformed.z < 1.f)
            vTransformed.z = 1.f;

        float ScreenCenterX = settings::system::screen_x / 2.0f;
        float ScreenCenterY = settings::system::screen_y / 2.0f;
        float ratio = (float)settings::system::screen_x / settings::system::screen_y;

        if (ratio < 4.0f / 3.0f)
            ratio = 4.0f / 3.0f;

        if (Zoom == 0)
            Zoom = 1;

        float fov = (ratio / (16.0f / 9.0f) * (float)tanf(cameraManager.FOV * (float)M_PI / 360.0f)) / Zoom;

        ScreenPosition->x = ScreenCenterX + vTransformed.x * ScreenCenterX / fov / vTransformed.z;
        ScreenPosition->y = ScreenCenterY - vTransformed.y * ScreenCenterX / fov / vTransformed.z;

        return ScreenPosition->x < settings::system::screen_x && ScreenPosition->x > 0 &&
            ScreenPosition->y < settings::system::screen_y && ScreenPosition->y > 0;
    }


    static void FNameDecrypt(char* inputBuf, int namelength)
    {
        if (namelength <= 0 || !inputBuf)
            return;

        uint8_t xor_key = mem.Read<uint8_t>(mem.BaseAddress + 0xB016D88);

        uint8_t* ptr = reinterpret_cast<uint8_t*>(inputBuf);

        uint8_t v4 = xor_key
            ^ ((xor_key >> 5) & 2)
            ^ (32 * ((xor_key ^ ((xor_key >> 5) & 2)) & 2));

        for (int i = 0; i < namelength; ++i)
        {
            ptr[i] = v4 ^ ptr[i] ^ ((v4 >> 5) & 2) ^ 0x4D;
        }
    }
    static void FFNameDecrypt(char* inputBuf, int namelength)
    {
        const char xor_key = mem.Read<BYTE>(mem.BaseAddress + 0xB011C08); // the decryption key
        char a2 = ((unsigned __int8)xor_key >> 5) & 2;

        if (namelength)
        {
            __int64 v3 = namelength;
            do
            {
                char v4 = xor_key ^ a2 ^ (32 * ((xor_key ^ a2) & 2));
                char result = v4 ^ *(BYTE*)(inputBuf) ^ ((v4 >> 5) & 2) ^ 0x55;
                *inputBuf = result;

                --v3;
                ++inputBuf;
                a2 = ((unsigned __int8)xor_key >> 5) & 2;
            } while (v3);
        }
    }
    std::string GetNameFromFName(int key)
    {
        auto chunkOffset = (UINT)((int)(key) >> 16);
        auto nameOffset = (USHORT)key;
        auto namePoolChunk = mem.Read<uintptr_t>(mem.BaseAddress + offsets::game::OFFSET_GNAMES + ((chunkOffset + 2) * 8));
        auto entryOffset = namePoolChunk + (ULONG)(2 * nameOffset);
        auto nameEntry = mem.Read<INT16>(entryOffset);
        auto nameLength = nameEntry >> 6;
        char buff[1028]{};
        if ((DWORD)nameLength && nameLength > 0)
        {
            mem.ReadString(entryOffset + 2, buff, nameLength);
            buff[nameLength] = '\0';
            FNameDecrypt(buff, nameLength); 
            return std::string(buff, nameLength);
        }
        else return "";
    }

 /*   std::string GetNNameFromFName(int key)
    {
        auto chunkOffset = (UINT)((int)(key) >> 16);
        auto nameOffset = (USHORT)key;
        auto namePoolChunk = mem.Read<uintptr_t>(mem.BaseAddress + offsets::game::OFFSET_GNAMES + ((chunkOffset + 2) * 8));
        auto entryOffset = namePoolChunk + (ULONG)(2 * nameOffset);
        auto nameEntry = mem.Read<INT16>(entryOffset);
        auto nameLength = nameEntry >> 6;
        char buff[1028]{};
        if ((DWORD)nameLength && nameLength > 0)
        {
            mem.ReadString(entryOffset + 2, buff, nameLength);
            buff[nameLength] = '\0';
            FNameDecrypt(buff, nameLength);
            return std::string(buff, nameLength);
        }
        else return "";
    }*/


    FVector ComponentToWorld(uintptr_t SkeletalMeshComponent) {
        uintptr_t c2w_ptr = mem.Read<uintptr_t>(SkeletalMeshComponent + offsets::USkeletalMeshComponent::ComponentToWorld);
        if (c2w_ptr) {
            return mem.Read<FVector>(c2w_ptr);
        }
    }

    struct USGCharacterEnduranceAttributeSet {
        float HeadLife;
        float HeadMaxLife;
        float ChestLife;
        float ChestMaxLife;
        float StomachLife;
        float StomachMaxLife;
        float LeftArmLife;
        float LeftArmMaxLife;
        float RightArmLife;
        float RightArmMaxLife;
        float LeftLegLife;
        float LeftLegMaxLife;
        float RightLegLife;
        float RightLegMaxLife;
    };

    float GetHealth(uintptr_t AbilitySystemComponent)
    {
        uint64_t SpawnedAttributes = mem.Read<uint64_t>(AbilitySystemComponent + 0x188); //AbilitySystemComponent->SpawnedAttributes
        uint64_t Health1 = mem.Read<uint64_t>(SpawnedAttributes + 0x08);
        USGCharacterEnduranceAttributeSet Health = mem.Read<USGCharacterEnduranceAttributeSet>(Health1 + 0x48);
        float HealthNow = (Health.HeadLife + Health.ChestLife + Health.StomachLife + Health.LeftArmLife + Health.RightArmLife + Health.LeftLegLife + Health.RightLegLife);
        float MaxHealth = (Health.HeadMaxLife + Health.ChestMaxLife + Health.StomachMaxLife + Health.LeftArmMaxLife + Health.RightArmMaxLife + Health.LeftLegMaxLife + Health.RightLegMaxLife);
        return HealthNow * 100.f / MaxHealth;
    }

   static f_transform Transformer(uintptr_t mesh, int index)
    {
        auto boneArray = mem.Read<uintptr_t>(mesh + 0x648);
        
        if (boneArray == 0)
            boneArray = mem.Read<uintptr_t>(mesh + 0x648 + 0x10);


        return mem.Read<f_transform>(boneArray + (uintptr_t)(index * 0x30));
    }

     static FVector GetbonePos(uintptr_t mesh, int id)
    {
        auto bone = Transformer(mesh, (int)id);
        auto FirstcomponentToWorld = mem.Read<uintptr_t>(mesh + offsets::USkeletalMeshComponent::ComponentToWorld); //WebOffsets.Dictionary["ComponentToWorld"]
        auto componentToWorld = mem.Read<f_transform>(FirstcomponentToWorld);
        auto matrix = bone.to_matrix().to_multiplication(componentToWorld.to_matrix()); ;
        return  FVector(matrix.w_plane.x, matrix.w_plane.y, matrix.w_plane.z);
    }
}