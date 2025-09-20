#include "triggerbot.h"
#include <windows.h>
#include <cmath>
#include <limits>
#include <iostream>
#include <thread>

namespace TriggerBot
{
    static bool g_bTriggerBotEnabled = false;
    void simulate_mouse_click()
    {
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }

    void Toggle()
    {
        static bool f4_was_pressed = false;
        bool f4_is_pressed = GetAsyncKeyState(VK_F4) & 0x8000;

        if (f4_is_pressed && !f4_was_pressed)
        {
            g_bTriggerBotEnabled = !g_bTriggerBotEnabled;
            if (g_bTriggerBotEnabled)
            {
                std::cout << "[+] TriggerBot Enabled. Hold Left Alt to activate. \r";
            }
            else
            {
                std::cout << "[-] TriggerBot Disabled.                          \r";
            }
        }
        f4_was_pressed = f4_is_pressed;
    }

    void Run(Entity &local, Entity &ent, int screen_width, int screen_height)
    {
        if (!g_bTriggerBotEnabled)
        {
            return;
        }

        bool aim_key_pressed = GetAsyncKeyState(VK_LMENU) & 0x8000;
        if (!aim_key_pressed)
        {
            return;
        }

        const int screenCenterX = screen_width / 2;
        const int screenCenterY = screen_height / 2;

        Entity const *target = nullptr;
        float min_dist_to_crosshair = std::numeric_limits<float>::max();

        if (ent.hp <= 0 || ent.team == local.team)
            return;

        float dist = std::sqrt(std::pow(ent.head2d.x - screenCenterX, 2) + std::pow(ent.head2d.y - screenCenterY, 2));
        if (dist < min_dist_to_crosshair)
        {
            min_dist_to_crosshair = dist;
            target = &ent;
        }

        if (target != nullptr)
        {
            float boxH = static_cast<float>(target->feet2d.y - target->head2d.y) * 0.2f;
            float boxW = boxH;
            float x = target->head2d.x - boxW / 2.0f;
            float y = target->head2d.y - boxH * 0.3f;

            if (screenCenterX >= x && screenCenterX <= x + boxW &&
                screenCenterY >= y && screenCenterY <= y + boxH)
            {
                simulate_mouse_click();
            }
        }
    }
}