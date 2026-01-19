#pragma once

#include <d3d.h>
#include <game/structs.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace math {
    D3DMATRIX Matrix(FRotator rotation) {
        float radPitch = rotation.pitch * (M_PI / 180.0f);
        float radYaw = rotation.yaw * (M_PI / 180.0f);
        float radRoll = rotation.roll * (M_PI / 180.0f);

        float SP = sinf(radPitch);
        float CP = cosf(radPitch);
        float SY = sinf(radYaw);
        float CY = cosf(radYaw);
        float SR = sinf(radRoll);
        float CR = cosf(radRoll);

        D3DMATRIX matrix;
        matrix._11 = CP * CY;
        matrix._12 = CP * SY;
        matrix._13 = SP;
        matrix._14 = 0.f;

        matrix._21 = SR * SP * CY - CR * SY;
        matrix._22 = SR * SP * SY + CR * CY;
        matrix._23 = -SR * CP;
        matrix._24 = 0.f;

        matrix._31 = -(CR * SP * CY + SR * SY);
        matrix._32 = CY * SR - CR * SP * SY;
        matrix._33 = CR * CP;
        matrix._34 = 0.f;

        matrix._41 = 0.f;
        matrix._42 = 0.f;
        matrix._43 = 0.f;
        matrix._44 = 1.f;

        return matrix;
    }
}