@echo off
rem =====================================================================
rem  Terminality - multi-toolchain build script
rem
rem  Usage:
rem    build.bat ^<msvc^|clang^|mingw^|all^> [Debug^|Release]
rem
rem  Examples:
rem    build.bat msvc Release     MSVC x64 Release
rem    build.bat clang            Clang x64 Debug (default configuration)
rem    build.bat mingw Release    MinGW GCC x64 Release
rem    build.bat all              every detected toolchain, Debug
rem
rem  Requires CMake 3.21+ and Ninja on PATH. The MSVC flow locates the
rem  newest Visual Studio installation itself via vswhere.exe.
rem =====================================================================
setlocal EnableDelayedExpansion

if "%~1"=="" goto usage
set TOOLCHAIN=%~1
set CONFIG=%~2
if "%CONFIG%"=="" set CONFIG=Debug

if /i not "%TOOLCHAIN%"=="msvc" if /i not "%TOOLCHAIN%"=="clang" if /i not "%TOOLCHAIN%"=="mingw" if /i not "%TOOLCHAIN%"=="all" goto usage
if /i not "%CONFIG%"=="Debug" if /i not "%CONFIG%"=="Release" goto usage

if /i "%TOOLCHAIN%"=="all" (
    set FAILED=
    for %%t in (msvc clang mingw) do (
        call :build_one %%t %CONFIG%
        if errorlevel 1 set FAILED=1
    )
    if defined FAILED (
        echo.
        echo Error: one or more toolchains failed to build %CONFIG%.
        exit /b 1
    )
    echo.
    echo Success: all toolchains built %CONFIG% successfully!
    exit /b 0
)

call :build_one %TOOLCHAIN% %CONFIG%
exit /b %errorlevel%

rem ---------------------------------------------------------------------
rem  Build a single toolchain/configuration pair via CMake presets
rem ---------------------------------------------------------------------
:build_one
setlocal
set TC=%~1
set CFG=%~2
set PRESET=%TC%-x64-

if /i "%CFG%"=="Debug" (set PRESET=%PRESET%debug) else (set PRESET=%PRESET%release)

echo.
echo =====================================================================
echo  Building preset "%PRESET%"
echo =====================================================================

if /i "%TC%"=="msvc" (
    call :ensure_msvc_env || exit /b 1
) else if /i "%TC%"=="clang" (
    where clang++ >nul 2>nul
    if errorlevel 1 (
        echo Error: clang++ not found on PATH.
        exit /b 1
    )
) else if /i "%TC%"=="mingw" (
    where g++ >nul 2>nul
    if errorlevel 1 (
        echo Error: g++ for MinGW not found on PATH.
        exit /b 1
    )
)

where ninja >nul 2>nul
if errorlevel 1 (
    echo Error: Ninja not found on PATH. Install it or use the copy bundled with Visual Studio.
    exit /b 1
)

cmake --preset %PRESET%
if errorlevel 1 (
    echo Error: CMake configuration failed for preset "%PRESET%".
    exit /b 1
)

cmake --build --preset %PRESET%
if errorlevel 1 (
    echo Error: Build failed for preset "%PRESET%".
    exit /b 1
)

echo Success: preset "%PRESET%" built.
exit /b 0

rem ---------------------------------------------------------------------
rem  Load the MSVC x64 environment via vswhere + vcvarsall (skip if the
rem  environment is already active in this session)
rem ---------------------------------------------------------------------
:ensure_msvc_env
if /i "%VSCMD_ARG_TGT_ARCH%"=="x64" exit /b 0

set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo Error: vswhere.exe not found. Visual Studio might not be installed.
    exit /b 1
)

set "VS_PATH="
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if "%VS_PATH%"=="" (
    echo Error: no Visual Studio installation with C++ tools found.
    exit /b 1
)

echo Found Visual Studio at: "%VS_PATH%"
set "VCVARS_BAT=%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat"
if not exist "%VCVARS_BAT%" (
    echo Error: vcvarsall.bat not found at "%VCVARS_BAT%"
    exit /b 1
)

call "%VCVARS_BAT%" x64 >nul
if errorlevel 1 (
    echo Error: failed to load the MSVC x64 environment.
    exit /b 1
)
exit /b 0

:usage
echo.
echo Usage: build.bat ^<msvc^|clang^|mingw^|all^> [Debug^|Release]
echo.
echo   msvc     Visual Studio C++ toolset (cl) - located via vswhere
echo   clang    LLVM Clang (clang++)           - must be on PATH
echo   mingw    MinGW-w64 GCC (g++)            - must be on PATH
echo   all      every detected toolchain
echo.
exit /b 1
