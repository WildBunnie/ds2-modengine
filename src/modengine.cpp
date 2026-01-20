#include "modengine.h"

#include "MinHook/MinHook.h"

#ifdef MODENGINE_DEBUG
#include <stdio.h>
#endif

#include <stdint.h>
#include <windows.h>

ds2_virtual_to_archive_path_t ds2_original_virtual_to_archive_path = 0;
ds2_grow_DLString_t ds2_grow_DLString = 0;

uintptr_t ds2_base_address = 0;
wchar_t user_defined_assets_dir[MAX_PATH] = {0};

int patch_memory(void* address, void* patch, size_t size)
{
    DWORD old_protect = 0;

    if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old_protect)) return 0;

    uint8_t* src = (uint8_t*)patch;
    uint8_t* dst = (uint8_t*)address;

    for (size_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }

    FlushInstructionCache(GetCurrentProcess(), address, size);
    VirtualProtect(address, size, old_protect, &old_protect);

    return 1;
}

int wstring_starts_with(const wchar_t* a, const wchar_t* b)
{
    if (a == b) return 1;
    if (!a || !b) return 0;

    while (*b) {
        if (*a != *b) return 0;
        if (*a == L'\0') return 0;
        a++;
        b++;
    }

    return 1;
}

size_t wstring_join_path(wchar_t* dest, wchar_t* part1, wchar_t* part2)
{
    size_t pos = 0;

    while (*part1 != '\0') {
        dest[pos++] = *part1++;
    }

    // append backslash if needed
    if (pos > 0 && dest[pos - 1] != L'\\' && dest[pos - 1] != L'/') {
        dest[pos++] = L'\\';
    }

    while (*part2 != '\0') {
        dest[pos++] = *part2++;
    }

    dest[pos] = L'\0';
    return pos;
}

#ifdef _M_IX86
// hooking thiscalls in 32 bit: https://www.unknowncheats.me/forum/617803-post5.html
uintptr_t __fastcall ds2_detour_virtual_to_archive_path(uintptr_t p1, void*, DLString* path)
#elif defined(_M_X64)
uintptr_t ds2_detour_virtual_to_archive_path(uintptr_t p1, DLString* path)
#endif
{
    // ignore capacity above 7 to not have to deal with SSO bullshit
    if (path && path->length > 0 && path->string) {
        // NOTE: i have no idea which things i actually need to override
        // overriding title:/ and gamedata:/ seems to be enough for now
        int prefix_len = -1;
        if (wstring_starts_with(path->string, L"title:/")) prefix_len = 7;
        if (wstring_starts_with(path->string, L"gamedata:/")) prefix_len = 10;

        if (prefix_len != -1) {
            wchar_t new_path[MAX_PATH] = {0};
            size_t new_path_len = wstring_join_path(
                new_path,
                user_defined_assets_dir,
                path->string + prefix_len
            );

            DWORD attrs = GetFileAttributesW(new_path);
            if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                if (path->capacity < (new_path_len + 1)) {
                    ds2_grow_DLString(path, (new_path_len + 1), path->length);
                }

                for (int i = 0; (path->string[i] = new_path[i]); i++);
                path->length = new_path_len;
            }
        }
    }

    return ds2_original_virtual_to_archive_path(p1, path);
}

void force_offline()
{
    UINT forceOffline = GetPrivateProfileIntW(
        L"online",
        L"forceOffline",
        1,
        L".\\ds2modengine.ini"
    );

    if (forceOffline != 1) return;

    // this is where the game calls getaddrinfo to try to resolve the server address
    // we simply change the if statement to always act as if it failed to resolve
    //
    // result = getaddrinfo(...,...,...,...);
    // if (result != 0) { <---- patches this
    //   ...
    //   return ...;
    // }

#ifdef _M_IX86
    uintptr_t address = ds2_base_address + 0xBB973;

    uint8_t patch[6] = {
        0xE9, 0x3A, 0x01, 0x00, 0x00,  // JMP DarkSoulsII.exe+0xBBAB2
        0x90                           // NOP
    };
#elif defined(_M_X64)
    uintptr_t address = ds2_base_address + 0xCD3887;

    uint8_t patch[6] = {
        0xE9, 0xF0, 0x00, 0x00, 0x00,  // JMP DarkSoulsII.exe+0xCD397C
        0x90                           // NOP
    };
#endif

    patch_memory((void*)address, patch, 6);
}

void patch_save_file()
{
    UINT useAlternativeSaveFile = GetPrivateProfileIntW(
        L"savefile",
        L"useAlternativeSaveFile",
        1,
        L".\\ds2modengine.ini"
    );

    if (useAlternativeSaveFile != 1) return;

    wchar_t buffer[4] = {0};
    wchar_t extension[4] = L"sl3";
    DWORD characters_read = GetPrivateProfileStringW(
        L"savefile",
        L"alternativeSaveFileExtension",
        extension,
        buffer,
        4,
        L".\\ds2modengine.ini"
    );

    if (characters_read == 3) {
        extension[0] = buffer[0];
        extension[1] = buffer[1];
        extension[2] = buffer[2];
        extension[3] = L'\0';
    }

    // +0x2 to skip the "." in ".sl2"
    uintptr_t address = ds2_base_address + DS2_DATA_save_file_extension + 0x2;

    patch_memory((void*)address, extension, 4 * sizeof(wchar_t));
}

void setup_asset_overrides()
{
    UINT useModOverrideDirectory = GetPrivateProfileIntW(
        L"files",
        L"useModOverrideDirectory",
        0,
        L".\\ds2modengine.ini"
    );

    if (useModOverrideDirectory != 1) return;

    GetPrivateProfileStringW(
        L"files",
        L"modOverrideDirectory",
        0,
        user_defined_assets_dir,
        MAX_PATH,
        L".\\ds2modengine.ini"
    );

    MH_CreateHook(
        (LPVOID)(ds2_base_address + DS2_FUNCTION_virtual_to_archive_path),
        (LPVOID)ds2_detour_virtual_to_archive_path,
        (LPVOID*)&ds2_original_virtual_to_archive_path
    );
}

void load_extra_dlls()
{
    UINT useExtraDLLsDirectory = GetPrivateProfileIntW(
        L"files",
        L"useExtraDLLsDirectory",
        0,
        L".\\ds2modengine.ini"
    );

    if (useExtraDLLsDirectory != 1) return;

    wchar_t dlls_dir[MAX_PATH] = {0};
    DWORD characters_read = GetPrivateProfileStringW(
        L"files",
        L"extraDllsDirectory",
        0,
        dlls_dir,
        MAX_PATH,
        L".\\ds2modengine.ini"
    );

    if (dlls_dir[0] == L'\0' || characters_read == 0) return;

    wchar_t search_path[MAX_PATH] = {0};
    wstring_join_path(search_path, dlls_dir, L"\\*.dll");

    WIN32_FIND_DATAW file_data;
    HANDLE find_handle = FindFirstFileW(search_path, &file_data);
    if (find_handle == INVALID_HANDLE_VALUE) return;

    do {
        if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        wchar_t dll_path[MAX_PATH] = {0};
        wstring_join_path(dll_path, dlls_dir, file_data.cFileName);
        LoadLibraryW(dll_path);
    } while (FindNextFileW(find_handle, &file_data));

    FindClose(find_handle);
}

// intro and press start stuff taken and adapted from
// https://github.com/r3sus/Resouls/tree/main/ds2s/mods
// TODO: understand and document this or rewrite it
void setup_qol_patches()
{
#ifdef _M_IX86
    uintptr_t no_logos_offset = 0x1146E96;
    uintptr_t no_press_start_offset = 0x1949B1;
#elif defined(_M_X64)
    uintptr_t no_logos_offset = 0x160DE1A;
    uintptr_t no_press_start_offset = 0xFDB66;
#endif

    UINT skipIntro = GetPrivateProfileIntW(
        L"qol",
        L"skipIntro",
        0,
        L".\\ds2modengine.ini"
    );

    if (skipIntro == 1) {
        uint8_t* address = (uint8_t*)(ds2_base_address + no_logos_offset);
        uint8_t patch = 0x1;
        patch_memory(address, &patch, 1);
    }

    UINT skipPressStart = GetPrivateProfileIntW(
        L"qol",
        L"skipPressStart",
        0,
        L".\\ds2modengine.ini"
    );

    if (skipPressStart == 1) {
        uint8_t* address = (uint8_t*)(ds2_base_address + no_press_start_offset);
        uint8_t patch = 0x2;
        patch_memory(address, &patch, 1);
    }
}

void modengine_run()
{
    // TODO: maybe wait for game to load
    MH_Initialize();
    ds2_base_address = (uintptr_t)GetModuleHandle(0);
    ds2_grow_DLString = (ds2_grow_DLString_t)(ds2_base_address + DS2_FUNCTION_grow_DLString);

#ifdef MODENGINE_DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
#endif

    force_offline();
    patch_save_file();
    setup_asset_overrides();
    load_extra_dlls();
    setup_qol_patches();

    MH_EnableHook(MH_ALL_HOOKS);
}