#include <stdint.h>

#ifdef _M_IX86
    #define DS2_DATA_save_file_extension 0xF389A4
    #define DS2_FUNCTION_virtual_to_archive_path 0x57EB70
    #define DS2_FUNCTION_grow_DLString 0x152F0
#elif defined(_M_X64)
    #define DS2_DATA_save_file_extension 0x11B5638
    #define DS2_FUNCTION_virtual_to_archive_path 0x89C5A0
    #define DS2_FUNCTION_grow_DLString 0x1BE60
#endif

// this is just a type of counted string used inside the game
// from the ?ServerName? discord server:
// | DLString stands for dantelion2 string
// | the full include path is "dantelion2/Core/Text/DLString.h"
// | what "dantelion2" is is up for debate
// capacity and length are measure in amount of characters not bytes
#ifdef _M_IX86
typedef struct {
    wchar_t* string;
    void* unk01;
    void* unk02;
    void* unk03;
    uint32_t length;
    uint32_t capacity;
    void* unk04;
} DLString;
#elif defined(_M_X64)
typedef struct {
    wchar_t* string;
    void* unk01;
    uint64_t length;
    uint64_t capacity;
} DLString;
#endif

typedef uintptr_t (__thiscall* ds2_virtual_to_archive_path_t)(uintptr_t, DLString*);
typedef void (__thiscall *ds2_grow_DLString_t)(DLString* string, size_t new_capacity, size_t old_length);

void modengine_run();