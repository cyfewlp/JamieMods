#include <iostream>
#include <windows.h>
using namespace std;

typedef VOID (*pfnUnhookKbdHook)();
typedef BOOL (*pfnSetKbdHook)();

int main()
{
    HMODULE          hMod = NULL;
    MSG              msg;
    pfnUnhookKbdHook pfnUnhook = NULL;
    pfnSetKbdHook    pfnHook   = NULL;

    hMod                       = LoadLibrary(L"GlobalHook.dll");
    if (NULL == hMod)
    {
        return (-1);
    }
    pfnHook = (pfnSetKbdHook)GetProcAddress(hMod, "SetKbdHook");
    if (NULL == pfnHook)
    {
        return (-1);
    }
    pfnHook();

    // 消息循环是必须的
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    pfnUnhook = (pfnUnhookKbdHook)GetProcAddress(hMod, "UnhookKbdHook");
    if (NULL == pfnUnhook)
    {
        return (-1);
    }
    pfnUnhook();

    system("pause");

    return (0);
}