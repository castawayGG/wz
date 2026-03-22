#include "app/app_controller.h"
#include "util/logger.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrev*/,
                    LPWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    // Ensure single instance
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"WzVirtualGamepadMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr,
            L"WZ Virtual Gamepad is already running.",
            L"WZ Virtual Gamepad", MB_ICONINFORMATION | MB_OK);
        return 0;
    }

    wz::AppController app;
    if (!app.Initialize(hInstance)) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        return 1;
    }

    app.Run();

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 0;
}
