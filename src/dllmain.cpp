#include "modengine.h"
#include <windows.h>

HINSTANCE dinput8_library_handle = {0};
LPCSTR dinput8_function_names[6] = { "DirectInput8Create", "DllCanUnloadNow", "DllGetClassObject", "DllRegisterServer", "DllUnregisterServer", "GetdfDIJoystick" };

#ifdef __cplusplus
extern "C" {
#endif
    UINT_PTR dinput8_original_functions[6] = {0};
    void DirectInput8Create_wrapper();
    void DllCanUnloadNow_wrapper();
    void DllGetClassObject_wrapper();
    void DllRegisterServer_wrapper();
    void DllUnregisterServer_wrapper();
    void GetdfDIJoystick_wrapper();
#ifdef __cplusplus
}
#endif

void load_original_dll()
{
    wchar_t buffer[MAX_PATH];
    wchar_t dll_path[] = L"\\dinput8.dll";

    UINT len = GetSystemDirectoryW(buffer, MAX_PATH);

    for (int i = 0; buffer[len] = dll_path[i]; i++, len++);

    dinput8_library_handle = LoadLibraryW(buffer);

    for (int i = 0; i < 6; i++) {
        dinput8_original_functions[i] = (UINT_PTR)GetProcAddress(
            dinput8_library_handle,
            dinput8_function_names[i]
        );
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            load_original_dll();
            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)modengine_run, 0, 0, 0);
            break;

        case DLL_THREAD_ATTACH: break;
        case DLL_THREAD_DETACH: break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
