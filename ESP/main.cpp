#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>
#include <vector>
#include <thread>
#include "esp.h"

uint64_t GetModuleBaseAddress(DWORD pid, const wchar_t* moduleName) {
    uint64_t base = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W me;
        me.dwSize = sizeof(me);
        if (Module32FirstW(snap, &me)) {
            do {
                if (_wcsicmp(me.szModule, moduleName) == 0) {
                    base = (uint64_t)me.modBaseAddr;
                    break;
                }
            } while (Module32NextW(snap, &me));
        }
        CloseHandle(snap);
    }
    return base;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); }
    return DefWindowProcW(hWnd, msg, wp, lp);
}

int main() {
    printf("[*] Starting C++ Box ESP\n");
    HWND hwnd = FindWindowW(NULL, L"Counter-Strike 2");
    if (!hwnd) { printf("[!] CS2 not running.\n"); return 1; }
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) { printf("[!] OpenProcess failed.\n"); return 1; }

    uint64_t base = GetModuleBaseAddress(pid, L"client.dll");
    if (!base) { printf("[!] Module base not found.\n"); return 1; }

    RECT screen;
    GetClientRect(GetDesktopWindow(), &screen);
    int width = screen.right;
    int height = screen.bottom;

    const wchar_t* clsName = L"CS2BoxESP";
    WNDCLASSW wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = clsName;
    RegisterClassW(&wc);

    HWND overlay = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        clsName, L"CS2 Box ESP",
        WS_POPUP, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL
    );
    SetLayeredWindowAttributes(overlay, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(overlay, SW_SHOW);

    HDC hdcWin = GetDC(overlay);
    HDC memDC = CreateCompatibleDC(hdcWin);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWin, width, height);
    SelectObject(memDC, hBitmap);

    MSG msg;
    float matrix[16];
    std::vector<Entity> ents;

    const int fps = 144;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        // clear
        HBRUSH brushBack = CreateSolidBrush(RGB(0, 0, 0));
        RECT r = { 0,0,width,height };
        FillRect(memDC, &r, brushBack);
        DeleteObject(brushBack);

        // read view matrix
        ReadProcessMemory(hProc, (LPCVOID)(base + Offsets::dwViewMatrix), matrix, sizeof(matrix), nullptr);

        ents = get_entities(hProc, base);
        for (auto& ent : ents) {
            if (ent.hp <= 0) continue;
            if (!ent.project(matrix, width, height)) continue;
            float boxH = static_cast<float>(ent.feet2d.y - ent.head2d.y) * 1.08f;
            float boxW = boxH / 2.0f;
            float x = ent.head2d.x - boxW / 2.0f;
            float y = ent.head2d.y - boxH * 0.08f;

            COLORREF col = (ent.team == 2) ? RGB(255, 0, 0) : RGB(0, 128, 255);
            HPEN pen = CreatePen(PS_SOLID, 1, col);
            SelectObject(memDC, pen);
            SelectObject(memDC, GetStockObject(NULL_BRUSH));
            Rectangle(memDC, (int)x, (int)y, (int)(x + boxW), (int)(y + boxH));
            DeleteObject(pen);

            float hp_ratio = ent.hp / 100.0f;
            float barH = boxH * hp_ratio;
            COLORREF hpcol = hp_ratio > 0.66f ? RGB(0, 255, 0) :
                hp_ratio > 0.33f ? RGB(255, 255, 0) : RGB(255, 0, 0);
            HBRUSH hpbrush = CreateSolidBrush(hpcol);
            RECT bar = { (int)(x - 5), (int)(y + (boxH - barH)), (int)(x - 2), (int)(y + boxH) };
            FillRect(memDC, &bar, hpbrush);
            DeleteObject(hpbrush);
        }

        BitBlt(hdcWin, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

        if (msg.message == WM_QUIT) break;
    }

    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(overlay, hdcWin);
    CloseHandle(hProc);

    return 0;
}
