#pragma once

#include <cstdint>
#define M_PI 3.14159265358979323846
enum class ECharacterType : int8_t
{
    ECharacterType_None = 0,
    ECharacterType_PMC = 1,
    ECharacterType_SCAV = 2,
    ECharacterType_AI_SCAV = 3,
    ECharacterType_AI_SCAV_BOSS = 4,
    ECharacterType_AI_PMC = 5,
    ECharacterType_AI_ELIT = 6,
    ECharacterType_BOSS = 7,
    ECharacterType_AI_SCAV_Follower = 8,
    ECharacterType_MAX = 9,
};

struct Bones{
    enum : int {
        Root = 0,
        Pelvis = 1,
        Spine_01 = 12,
        Spine_02 = 13,
        Spine_03 = 14,
        Neck = 15,
        Head = 16,

        // Left Leg
        Thigh_L = 2,
        Calf_L = 4,
        Foot_L = 5,

        // Right Leg
        Thigh_R = 7,
        Calf_R = 9,
        Foot_R = 10,

        // Left Arm
        Clavicle_L = 50,
        UpperArm_L = 51,
        LowerArm_L = 52,
        Hand_L = 54,

        // Right Arm
        Clavicle_R = 20,
        UpperArm_R = 21,
        LowerArm_R = 22,
        Hand_R = 24,

        // Hands
        Item_R = 40,
        Item_L = 70,

        // Camera
        Camera = 80
    };
};

struct FVector {
    float x, y, z;

    FVector operator-(const FVector& other) const {
        return FVector{ x - other.x, y - other.y, z - other.z };
    }

    float Dot(const FVector& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    float Distance(const FVector& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;

        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
};

struct FRotator {
    float pitch, yaw, roll;

    FRotator operator-(const FRotator& other) const {
        return FRotator{ pitch - other.pitch, yaw - other.yaw, roll - other.roll };
    }
};

struct f_plane : FVector {

    f_plane() : w(0) {}
    f_plane(double w) : w(w) {}

    FVector to_vector() {
        FVector value;
        value.x = this->x;
        value.y = this->y;
        value.z = this->z;
        return value;
    }

    double w;
};

class f_matrix {
public:
    double m[4][4];
    f_plane x_plane, y_plane, z_plane, w_plane;

    f_matrix() : x_plane(), y_plane(), z_plane(), w_plane() {}
    f_matrix(f_plane x_plane, f_plane y_plane, f_plane z_plane, f_plane w_plane)
        : x_plane(x_plane), y_plane(y_plane), z_plane(z_plane), w_plane(w_plane) {
    }
    f_matrix(FRotator& rotator) : x_plane(), y_plane(), z_plane(), w_plane() {
        this->to_rotation_matrix(rotator);
    }

    f_matrix to_rotation_matrix(FRotator& rotation) {
        f_matrix matrix = {};

        auto rad_pitch = (rotation.pitch * M_PI / 180.f);
        auto rad_yaw = (rotation.yaw * M_PI / 180.f);
        auto rad_roll = (rotation.roll * M_PI / 180.f);

        auto sin_pitch = sin(rad_pitch);
        auto cos_pitch = cos(rad_pitch);

        auto sin_yaw = sin(rad_yaw);
        auto cos_yaw = cos(rad_yaw);

        auto sin_roll = sin(rad_roll);
        auto cos_roll = cos(rad_roll);

        matrix.x_plane.x = cos_pitch * cos_yaw;
        matrix.x_plane.y = cos_pitch * sin_yaw;
        matrix.x_plane.z = sin_pitch;
        matrix.x_plane.w = 0.f;

        matrix.y_plane.x = sin_roll * sin_pitch * cos_yaw - cos_roll * sin_yaw;
        matrix.y_plane.y = sin_roll * sin_pitch * sin_yaw + cos_roll * cos_yaw;
        matrix.y_plane.z = -sin_roll * cos_pitch;
        matrix.y_plane.w = 0.f;

        matrix.z_plane.x = -(cos_roll * sin_pitch * cos_yaw + sin_roll * sin_yaw);
        matrix.z_plane.y = cos_yaw * sin_roll - cos_roll * sin_pitch * sin_yaw;
        matrix.z_plane.z = cos_roll * cos_pitch;
        matrix.z_plane.w = 0.f;

        matrix.w_plane.w = 1.f;
        return matrix;
    }

    f_matrix to_multiplication(f_matrix m_matrix) const {
        f_matrix matrix{};

        matrix.w_plane.x = (
            this->w_plane.x * m_matrix.x_plane.x +
            this->w_plane.y * m_matrix.y_plane.x +
            this->w_plane.z * m_matrix.z_plane.x +
            this->w_plane.w * m_matrix.w_plane.x
            );

        matrix.w_plane.y = (
            this->w_plane.x * m_matrix.x_plane.y +
            this->w_plane.y * m_matrix.y_plane.y +
            this->w_plane.z * m_matrix.z_plane.y +
            this->w_plane.w * m_matrix.w_plane.y
            );

        matrix.w_plane.z = (
            this->w_plane.x * m_matrix.x_plane.z +
            this->w_plane.y * m_matrix.y_plane.z +
            this->w_plane.z * m_matrix.z_plane.z +
            this->w_plane.w * m_matrix.w_plane.z
            );

        matrix.w_plane.w = (
            this->w_plane.x * m_matrix.x_plane.w +
            this->w_plane.y * m_matrix.y_plane.w +
            this->w_plane.z * m_matrix.z_plane.w +
            this->w_plane.w * m_matrix.w_plane.w
            );

        return matrix;
    }
};

class f_transform {
public:
    f_plane rotation;
    FVector translation;
    char pad[0x4];
    FVector scale;
    char pad1[0x4];

    f_transform() : rotation(), translation(0.f, 0.f, 0.f), scale(0.f, 0.f, 0.f), pad(), pad1() {}

    f_transform(const f_plane& rot, const FVector& translation, const FVector& scale) {
        this->rotation = rot;
        this->translation = translation;

        this->pad[0x4] = 0;
        this->scale = scale;
        this->pad1[0x4] = 0;
    }

    void fix_scaler() {
        if (this->scale.x == 0) this->scale.x = 1;
        if (this->scale.y == 0) this->scale.y = 1;
        if (this->scale.z == 0) this->scale.z = 1;
    }

    f_matrix to_matrix() {
        f_matrix matrix = {};

        auto x2 = this->rotation.x * 2;
        auto y2 = this->rotation.y * 2;
        auto z2 = this->rotation.z * 2;

        auto xx2 = this->rotation.x * x2;
        auto yy2 = this->rotation.y * y2;
        auto zz2 = this->rotation.z * z2;

        auto yz2 = this->rotation.y * z2;
        auto wx2 = this->rotation.w * x2;

        auto xy2 = this->rotation.x * y2;
        auto wz2 = this->rotation.w * z2;

        auto xz2 = this->rotation.x * z2;
        auto wy2 = this->rotation.w * y2;

        matrix.x_plane.x = (1.0 - (yy2 + zz2)) * this->scale.x;
        matrix.x_plane.y = (xy2 + wz2) * this->scale.x;
        matrix.x_plane.z = (xz2 - wy2) * this->scale.x;

        matrix.y_plane.x = (xy2 - wz2) * this->scale.y;
        matrix.y_plane.y = (1.0 - (xx2 + zz2)) * this->scale.y;
        matrix.y_plane.z = (yz2 + wx2) * this->scale.y;

        matrix.z_plane.x = (xz2 + wy2) * this->scale.z;
        matrix.z_plane.y = (yz2 - wx2) * this->scale.z;
        matrix.z_plane.z = (1.0 - (xx2 + yy2)) * this->scale.z;

        matrix.w_plane.x = this->translation.x;
        matrix.w_plane.y = this->translation.y;
        matrix.w_plane.z = this->translation.z;

        matrix.w_plane.w = 1.0;

        return matrix;
    }
};
struct FVector2D {
    int x, y;
};

struct FMinimalViewInfo {
    FVector Location;      
    FRotator Rotation;     
    float FOV;             
    float ShadowFOV;       
    float DesiredFOV;      
    float OrthoWidth;
};

struct Player {
    uintptr_t actor_mesh;
    uintptr_t actor_state;
    uintptr_t actor_pawn;
    uintptr_t actor_rootcomponent;
    uint32_t actor_id;
    std::string actor_name;
    std::string player_name;
    float health;
    bool bot = false;
    bool IsFriend = false;
    bool is_dead = false;
    FVector head_pos;
};
struct Loot {
    uintptr_t actor_mesh;
    uintptr_t actor_state;
    uintptr_t actor_pawn;
    uintptr_t actor_rootcomponent;
    uint32_t actor_id;
    std::string actor_name;
    std::string player_name;
    int price;

};