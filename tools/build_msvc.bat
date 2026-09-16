@echo off
setlocal

set "CONFIGURATION=%~1"
if "%CONFIGURATION%"=="" set "CONFIGURATION=Debug"

set "TARGET=%~2"
if "%TARGET%"=="" set "TARGET=SDL2_GameEngine"

if not defined VCPKG_ROOT (
    if exist "C:\Users\alex\Tools\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_ROOT=C:\Users\alex\Tools\vcpkg"
    ) else if exist "C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_ROOT=C:\dev\vcpkg"
    )
)

if not defined VCPKG_ROOT (
    echo VCPKG_ROOT is not set and no default vcpkg checkout was found.
    exit /b 1
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_INSTALL=%%I"
    )
)

if not defined VS_INSTALL (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
        set "VS_INSTALL=C:\Program Files\Microsoft Visual Studio\2022\Community"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" (
        set "VS_INSTALL=C:\Program Files\Microsoft Visual Studio\18\Community"
    )
)

if not defined VS_INSTALL (
    echo Visual Studio with the C++ toolchain was not found.
    exit /b 1
)

set "VS_DEV_CMD=%VS_INSTALL%\Common7\Tools\VsDevCmd.bat"
set "CMAKE_EXE=%VS_INSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

if not exist "%VS_DEV_CMD%" (
    echo Visual Studio developer command prompt was not found: %VS_DEV_CMD%
    exit /b 1
)

if not exist "%CMAKE_EXE%" (
    echo Visual Studio CMake was not found: %CMAKE_EXE%
    exit /b 1
)

call "%VS_DEV_CMD%" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%

"%CMAKE_EXE%" --preset msvc-vs2022
if errorlevel 1 exit /b %errorlevel%

if /I "%CONFIGURATION%"=="Release" (
    "%CMAKE_EXE%" --build --preset msvc-release --target "%TARGET%" --parallel
) else (
    "%CMAKE_EXE%" --build --preset msvc-debug --target "%TARGET%" --parallel
)

exit /b %errorlevel%
