@echo off
setlocal

REM Default mode is local
set MODE=%1
if "%MODE%"=="" set MODE=local

REM Default configuration
set CONFIG=Release

REM Get script directory
set SCRIPT_DIR=%~dp0

REM Search for vcpkg in parent directories
set VCPKG_TOOLCHAIN=
set SEARCH_DIR=%SCRIPT_DIR%
:find_vcpkg
if exist "%SEARCH_DIR%vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set VCPKG_TOOLCHAIN=%SEARCH_DIR%vcpkg\scripts\buildsystems\vcpkg.cmake
) else (
    REM Go up one directory
    cd "%SEARCH_DIR%"
    cd ..
    set SEARCH_DIR=%CD%\
    if "%SEARCH_DIR%"=="%SEARCH_DIR:~0,3%\" (
        REM Reached root, stop searching
        goto :vcpkg_not_found
    )
    goto find_vcpkg
)

if "%VCPKG_TOOLCHAIN%"=="" (
    :vcpkg_not_found
    echo Could not find vcpkg toolchain file. Make sure vcpkg is installed.
    pause
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist build mkdir build
set BUILD_DIR=%SCRIPT_DIR%build
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"


REM Configure CMake based on mode
if /I "%MODE%"=="client" (
    cmake .. -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF -DBUILD_LOCAL=OFF -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN%
) else if /I "%MODE%"=="server" (
    cmake .. -DBUILD_CLIENT=OFF -DBUILD_SERVER=ON -DBUILD_LOCAL=OFF -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN%
) else if /I "%MODE%"=="local" (
    cmake .. -DBUILD_LOCAL=ON -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF -DCMAKE_TOOLCHAIN_FILE=%VCPKG_TOOLCHAIN%
) else (
    echo Usage: build.bat [client^|server^|local]
    goto :eof
)
if errorlevel 1 goto :error

REM Build
cmake --build . --config %CONFIG%
if errorlevel 1 goto :error

cd ..

REM Run the executable for client/local mode
if /I "%MODE%"=="client" (
    if exist %CONFIG%\GlCraftClient.exe (
        %CONFIG%\GlCraftClient.exe
    )
) else if /I "%MODE%"=="local" (
    if exist %CONFIG%\GlCraftLocal.exe (
        %CONFIG%\GlCraftLocal.exe
    )
) else if /I "%MODE%"=="server" (
    if exist %CONFIG%\GlCraftServer.exe (
        %CONFIG%\GlCraftServer.exe
    )
)

goto :eof

:error
echo An error occurred.
pause
