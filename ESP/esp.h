#pragma once
#include <windows.h>
#include <vector>

// Keep your Offsets, Vec3, Entity structs here:
struct Offsets {
    static constexpr uint64_t dwEntityList = 30139936;
    static constexpr uint64_t dwLocalPlayerController = 30474816;
    static constexpr uint64_t dwViewMatrix = 30546432;
    static constexpr uint64_t m_hPlayerPawn = 2300;
    static constexpr uint64_t m_iHealth = 844;
    static constexpr uint64_t m_iTeamNum = 1003;
    static constexpr uint64_t m_pBoneArray = 496;
    static constexpr uint64_t m_pGameSceneNode = 816;
    static constexpr uint64_t m_vOldOrigin = 5552;
    static constexpr uint64_t m_pClippingWeapon = 5664;
    static constexpr uint64_t m_AttributeManager = 5024;
    static constexpr uint64_t m_Item = 80;
    static constexpr uint64_t m_iItemDefinitionIndex = 442;
    static constexpr uint64_t m_iszPlayerName = 1768;

};

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

// Functions declared for ESP logic
std::vector<Entity> get_entities(HANDLE hProcess, uint64_t base);
