@echo off

if "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    set ARCH=64
    set ASSEMBLER=ml64
) else if "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    set ARCH=32
    set ASSEMBLER=ml
) else (
    echo MSVC environment not initialized
    exit /b
)

mkdir build\%ARCH%
pushd build\%ARCH%
%ASSEMBLER% /Fo dinput8.obj /c ../../src/dinput8_%ARCH%.asm
cl ../../src/dllmain.cpp ../../src/modengine.cpp ../../src/dinput8.def dinput8.obj ^
    ../../dependencies/MinHook/lib/libMinHook.%VSCMD_ARG_TGT_ARCH%.lib ^
    /LD /MT ^
    /I"../../dependencies" ^
    /Fe:dinput8.dll
popd