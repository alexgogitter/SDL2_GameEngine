@echo off
setlocal

set "CONFIGURATION=%~1"
if "%CONFIGURATION%"=="" set "CONFIGURATION=Debug"

set "TARGET=%~2"
if "%TARGET%"=="" set "TARGET=SDL2_GameEngine"

rem This project preset is explicitly for Visual Studio 2022. Do not let
rem vswhere choose Visual Studio 2026, because that leaves a VS 18 generator
rem instance in the CMake cache and breaks the VS 17 2022 preset.
set "VS_INSTALL=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "VS_DEV_CMD=%VS_INSTALL%\Common7\Tools\VsDevCmd.bat"
set "CMAKE_EXE=%VS_INSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

rem Use the standalone vcpkg checkout. VsDevCmd may set VCPKG_ROOT to Visual
rem Studio's bundled vcpkg, so we set this again after calling VsDevCmd.
set "PROJECT_VCPKG_ROOT=C:\Users\alex\Tools\vcpkg"

if not exist "%VS_DEV_CMD%" (
    echo Visual Studio 2022 developer command prompt was not found:
    echo   %VS_DEV_CMD%
    exit /b 1
)

if not exist "%CMAKE_EXE%" (
    echo Visual Studio 2022 CMake was not found:
    echo   %CMAKE_EXE%
    exit /b 1
)

if not exist "%PROJECT_VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    echo Standalone vcpkg was not found:
    echo   %PROJECT_VCPKG_ROOT%
    exit /b 1
)

call "%VS_DEV_CMD%" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b %errorlevel%

set "VCPKG_ROOT=%PROJECT_VCPKG_ROOT%"

rem --fresh clears stale CMake cache entries such as an old VS 18 instance
rem while keeping the generated vcpkg packages under the build directory.
"%CMAKE_EXE%" --fresh --preset msvc-vs2022
if errorlevel 1 exit /b %errorlevel%

if /I "%CONFIGURATION%"=="Release" (
    "%CMAKE_EXE%" --build --preset msvc-release --target "%TARGET%" --parallel
) else (
    "%CMAKE_EXE%" --build --preset msvc-debug --target "%TARGET%" --parallel
)

exit /b %errorlevel%
