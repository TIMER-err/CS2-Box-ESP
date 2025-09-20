#include "esp.h"
#include <windows.h>
#include <vector>
#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

Offsets offsets;

bool load_offsets() {
    try {
        // 加载 offsets.json
        std::ifstream offsets_file("offsets.json");
        if (!offsets_file.is_open()) {
            std::cerr << "错误: 无法打开 offsets.json" << std::endl;
            return false;
        }
        json offsets_json = json::parse(offsets_file);
        offsets_file.close();

        // 加载 client_dll.json
        std::ifstream client_dll_file("client_dll.json");
        if (!client_dll_file.is_open()) {
            std::cerr << "错误: 无法打开 client_dll.json" << std::endl;
            return false;
        }
        json client_dll_json = json::parse(client_dll_file);
        client_dll_file.close();

        // 从 offsets.json 赋值
        offsets.dwEntityList = offsets_json["client.dll"]["dwEntityList"];
        offsets.dwLocalPlayerController = offsets_json["client.dll"]["dwLocalPlayerController"];
        offsets.dwViewMatrix = offsets_json["client.dll"]["dwViewMatrix"];

        // 从 client_dll.json 赋值
        offsets.m_hPlayerPawn = client_dll_json["client.dll"]["classes"]["CCSPlayerController"]["fields"]["m_hPlayerPawn"];
        offsets.m_iHealth = client_dll_json["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iHealth"];
        offsets.m_iTeamNum = client_dll_json["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iTeamNum"];
        offsets.m_pGameSceneNode = client_dll_json["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_pGameSceneNode"];
        offsets.m_modelState = client_dll_json["client.dll"]["classes"]["CSkeletonInstance"]["fields"]["m_modelState"];
        offsets.m_vOldOrigin = client_dll_json["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"];
        offsets.m_pClippingWeapon = client_dll_json["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_pClippingWeapon"];
        offsets.m_AttributeManager = client_dll_json["client.dll"]["classes"]["C_Chicken"]["fields"]["m_AttributeManager"];
        offsets.m_Item = client_dll_json["client.dll"]["classes"]["C_AttributeContainer"]["fields"]["m_Item"];
        offsets.m_iItemDefinitionIndex = client_dll_json["client.dll"]["classes"]["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"];
        offsets.m_iszPlayerName = client_dll_json["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_iszPlayerName"];

        // 计算派生偏移量
        offsets.m_pBoneArray = offsets.m_modelState + 0x80;

    } catch (const json::parse_error& e) {
        std::cerr << "JSON 解析错误: " << e.what() << std::endl;
        return false;
    } catch (const json::exception& e) {
        std::cerr << "JSON 错误 (可能是键名不存在): " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "加载偏移量时发生未知错误。" << std::endl;
        return false;
    }

    std::cout << "偏移量已成功加载。" << std::endl;
    return true;
}

bool Entity::read_data() {
    SIZE_T bytes;
    uint32_t tmp;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(pawn + offsets.m_iHealth), &tmp, sizeof(tmp), &bytes)) return false;
    hp = (int)tmp;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(pawn + offsets.m_iTeamNum), &tmp, sizeof(tmp), &bytes)) return false;
    team = (int)tmp;
    if (!ReadProcessMemory(hProcess, (LPCVOID)(pawn + offsets.m_vOldOrigin), &pos, sizeof(pos), &bytes)) return false;
    uint64_t scene = 0;
    ReadProcessMemory(hProcess, (LPCVOID)(pawn + offsets.m_pGameSceneNode), &scene, sizeof(scene), &bytes);
    uint64_t bones = 0;
    ReadProcessMemory(hProcess, (LPCVOID)(scene + offsets.m_pBoneArray), &bones, sizeof(bones), &bytes);
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
    ReadProcessMemory(hProcess, (LPCVOID)(base + offsets.dwLocalPlayerController), &local, sizeof(local), nullptr);
    ReadProcessMemory(hProcess, (LPCVOID)(base + offsets.dwEntityList), &listPtr, sizeof(listPtr), nullptr);

    for (int i = 1; i <= 64; ++i) {
        uint64_t entry = 0, ctrl = 0, pawnPtr = 0, pawn = 0;
        ReadProcessMemory(hProcess, (LPCVOID)(listPtr + (8 * (i & 0x7FFF) >> 9) + 16), &entry, sizeof(entry), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(entry + 120 * (i & 0x1FF)), &ctrl, sizeof(ctrl), nullptr);
        if (!ctrl || ctrl == local) continue;
        ReadProcessMemory(hProcess, (LPCVOID)(ctrl + offsets.m_hPlayerPawn), &pawnPtr, sizeof(pawnPtr), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(listPtr + 8 * ((pawnPtr & 0x7FFF) >> 9) + 16), &pawn, sizeof(pawn), nullptr);
        ReadProcessMemory(hProcess, (LPCVOID)(pawn + 120 * (pawnPtr & 0x1FF)), &pawn, sizeof(pawn), nullptr);
        if (!pawn) continue;

        Entity e{ hProcess, ctrl, pawn };
        if (e.read_data()) ents.push_back(e);
    }
    return ents;
}