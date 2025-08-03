#include "esp.h"
#include <windows.h>
#include <vector>

bool Entity::read_data() {
    SIZE_T bytes;
    uint32_t tmp;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(pawn + Offsets::m_iHealth), &tmp, sizeof(tmp), &bytes)) return false;
    hp = (int)tmp;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(pawn + Offsets::m_iTeamNum), &tmp, sizeof(tmp), &bytes)) return false;
    team = (int)tmp;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(pawn + Offsets::m_vOldOrigin), &pos, sizeof(pos), &bytes)) return false;

    uint64_t scene = 0;
    ReadProcessMemory(hProcess, (LPCVOID)(pawn + Offsets::m_pGameSceneNode), &scene, sizeof(scene), &bytes);
    uint64_t bones = 0;
    ReadProcessMemory(hProcess, (LPCVOID)(scene + Offsets::m_pBoneArray), &bones, sizeof(bones), &bytes);
    Vec3 bone6;
    ReadProcessMemory(hProcess, (LPCVOID)(bones + 6 * 32), &bone6, sizeof(bone6), &bytes);
    head = bone6;
    return true;
}

bool Entity::world_to_screen(const float matrix[16], int width, int height, Vec3 const& v, POINT& out) {
    float x = matrix[0] * v.x + matrix[1] * v.y + matrix[2] * v.z + matrix[3];
    float y = matrix[4] * v.x + matrix[5] * v.y + matrix[6] * v.z + matrix[7];
    float w = matrix[12] * v.x + matrix[13] * v.y + matrix[14] * v.z + matrix[15];
    if (w < 0.1f) return false;
    float invw = 1.0f / w;
    float sx = width / 2.0f + (x * invw) * width / 2.0f;
    float sy = height / 2.0f - (y * invw) * height / 2.0f;
    out.x = (LONG)sx;
    out.y = (LONG)sy;
    return true;
}

bool Entity::project(const float matrix[16], int width, int height) {
    if (!world_to_screen(matrix, width, height, pos, feet2d)) return false;
    if (!world_to_screen(matrix, width, height, head, head2d)) return false;
    return true;
}

std::vector<Entity> get_entities(HANDLE hProcess, uint64_t base) {
    std::vector<Entity> ents;
    uint64_t local = 0, listPtr = 0;
    ReadProcessMemory(hProcess, (LPCVOID)(base + Offsets::dwLocalPlayerController), &local, sizeof(local), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(base + Offsets::dwEntityList), &listPtr, sizeof(listPtr), nullptr);

    for (int i = 1; i <= 64; ++i) {
        uint64_t entry = 0, ctrl = 0, pawnPtr = 0, pawn = 0;
        ReadProcessMemory(hProcess, (LPCVOID)(listPtr + (8 * (i & 0x7FFF) >> 9) + 16), &entry, sizeof(entry), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(entry + 120 * (i & 0x1FF)), &ctrl, sizeof(ctrl), nullptr);
        if (!ctrl || ctrl == local) continue;
        ReadProcessMemory(hProcess, (LPCVOID)(ctrl + Offsets::m_hPlayerPawn), &pawnPtr, sizeof(pawnPtr), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(listPtr + 8 * ((pawnPtr & 0x7FFF) >> 9) + 16), &pawn, sizeof(pawn), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(pawn + 120 * (pawnPtr & 0x1FF)), &pawn, sizeof(pawn), nullptr);
        if (!pawn) continue;

        Entity e{ hProcess, ctrl, pawn };
        if (e.read_data()) ents.push_back(e);
    }
    return ents;
}
