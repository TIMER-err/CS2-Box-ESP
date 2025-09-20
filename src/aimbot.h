#pragma once

#include "basic.h"
#include <vector>

namespace Aimbot
{
    void Toggle();
    void Run(HANDLE hProcess, uint64_t base, Entity &local, std::vector<Entity> &entities, int screen_width, int screen_height, float matrix[16]);
}