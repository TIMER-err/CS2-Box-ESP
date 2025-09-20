#include "aimbot.h"
#include <windows.h>
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>

namespace Aimbot
{
    // =========================================================================================
    // Aimbot 配置项
    // =========================================================================================
    static bool g_bAimbotEnabled = false;
    const float AIM_FOV = 5.0f;          // Aimbot生效的最大角度范围 (单位: 度)
    const float SMOOTH_FACTOR = 0.85f;   // 平滑系数: 0.0 (无平滑) 到 0.99 (极度平滑)
    const int RCS_START_BULLET = 1;      // 第几发子弹后开始控制后坐力
    const Vec2 RCS_SCALE = {2.0f, 2.0f}; // 后坐力补偿比例 (x: Pitch, y: Yaw)
    // =========================================================================================

    const double PI = 3.14159265358979323846;

    // 切换 Aimbot 启用/禁用状态
    void Toggle()
    {
        static bool f5_was_pressed = false;
        bool f5_is_pressed = GetAsyncKeyState(VK_F5) & 0x8000;
        if (f5_is_pressed && !f5_was_pressed)
        {
            g_bAimbotEnabled = !g_bAimbotEnabled;
            if (g_bAimbotEnabled)
            {
                std::cout << "[+] Aimbot Enabled.    Hold Left Alt to activate. \r";
            }
            else
            {
                std::cout << "[-] Aimbot Disabled.                              \r";
            }
        }
        f5_was_pressed = f5_is_pressed;
    }

    // 将角度规范化到 [-180, 180] 范围
    float normalize_angle(float angle)
    {
        while (angle > 180.0f)
            angle -= 360.0f;
        while (angle < -180.0f)
            angle += 360.0f;
        return angle;
    }

    float clamp(float val, float min_val, float max_val)
    {
        if (val < min_val)
            return min_val;
        if (val > max_val)
            return max_val;
        return val;
    }

    // 直接写入视角角度到内存
    void SetViewAngle(HANDLE hProcess, uint64_t base, float yaw, float pitch)
    {
        if (!base)
            return;

        pitch = clamp(pitch, -89.0f, 89.0f);
        yaw = normalize_angle(yaw);

        Vec2 new_angles = {pitch, yaw};
        WriteProcessMemory(hProcess, (LPVOID)(base + offsets.dwViewAngles), &new_angles, sizeof(new_angles), nullptr);
    }

    // Aimbot 核心运行逻辑
    void Run(HANDLE hProcess, uint64_t base, Entity &local, std::vector<Entity> &entities, int screen_width, int screen_height, float matrix[16])
    {
        if (!g_bAimbotEnabled || !(GetAsyncKeyState(VK_LMENU) & 0x8000))
        {
            return;
        }

        const int screenCenterX = screen_width / 2;
        const int screenCenterY = screen_height / 2;
        Entity *target = nullptr;
        float min_dist_to_crosshair = std::numeric_limits<float>::max();

        // 1. 寻找离准星最近的敌人
        for (auto &ent : entities)
        {
            if (ent.hp <= 0 || ent.team == local.team)
                continue;
            if (!ent.project(matrix, screen_width, screen_height))
                continue;
            float dist = std::sqrt(std::pow(ent.head2d.x - screenCenterX, 2) + std::pow(ent.head2d.y - screenCenterY, 2));
            if (dist < min_dist_to_crosshair)
            {
                min_dist_to_crosshair = dist;
                target = &ent;
            }
        }

        if (target == nullptr)
            return;

        // 2. 计算目标角度
        Vec3 delta_vec = {target->head.x - local.head.x, target->head.y - local.head.y, target->head.z - local.head.z};
        float distance = std::sqrt(delta_vec.x * delta_vec.x + delta_vec.y * delta_vec.y + delta_vec.z * delta_vec.z);
        float required_yaw = static_cast<float>(atan2(delta_vec.y, delta_vec.x) * (180.0 / PI));
        float required_pitch = static_cast<float>(-atan(delta_vec.z / distance) * (180.0 / PI));

        // 3. FOV 检查
        float delta_yaw_check = normalize_angle(required_yaw - local.angles.y);
        float delta_pitch_check = normalize_angle(required_pitch - local.angles.x);
        if (std::sqrt(delta_yaw_check * delta_yaw_check + delta_pitch_check * delta_pitch_check) > AIM_FOV)
        {
            return;
        }

        // 4. 计算平滑后的新角度
        float new_yaw = local.angles.y + delta_yaw_check * (1.0f - SMOOTH_FACTOR);
        float new_pitch = local.angles.x + delta_pitch_check * (1.0f - SMOOTH_FACTOR);

        // 5. 应用后坐力控制 (RCS)
        if (local.shotsFired > RCS_START_BULLET)
        {
            new_pitch -= local.aimPunch.x * RCS_SCALE.x;
            new_yaw -= local.aimPunch.y * RCS_SCALE.y;
        }

        // 6. 将最终角度写入内存
        SetViewAngle(hProcess, base, new_yaw, new_pitch);
    }
}