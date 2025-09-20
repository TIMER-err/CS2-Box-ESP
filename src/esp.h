#pragma once
#include <windows.h>
#include <vector>
#include <cstdint>
#include <string>

struct Offsets {
    uint64_t dwEntityList;
    uint64_t dwLocalPlayerController;
    uint64_t dwViewMatrix;
    uint64_t m_hPlayerPawn;
    uint64_t m_iHealth;
    uint64_t m_iTeamNum;
    uint64_t m_modelState;
    uint64_t m_pBoneArray;
    uint64_t m_pGameSceneNode;
    uint64_t m_vOldOrigin;
    uint64_t m_pClippingWeapon;
    uint64_t m_AttributeManager;
    uint64_t m_Item;
    uint64_t m_iItemDefinitionIndex;
    uint64_t m_iszPlayerName;
};

extern Offsets offsets;

bool load_offsets();

struct Vec3 {
    float x, y, z;
};

struct Entity {
    HANDLE hProcess;
    uint64_t controller, pawn;
    int hp;
    int team;
    Vec3 pos, head;
    POINT head2d, feet2d;

    bool read_data();
    bool world_to_screen(const float matrix[16], int width, int height, Vec3 const& v, POINT& out);
    bool project(const float matrix[16], int width, int height);
};

std::vector<Entity> get_entities(HANDLE hProcess, uint64_t base);