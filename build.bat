@echo off
setlocal

REM Default mode is local
set MODE=%1
if "%MODE%"=="" set MODE=local

REM Default configuration
set CONFIG=Release

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure CMake based on mode
if /I "%MODE%"=="client" (
    cmake .. -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF -DBUILD_LOCAL=OFF -DCMAKE_TOOLCHAIN_FILE=C:/Users/Lakshay/vcpkg/scripts/buildsystems/vcpkg.cmake
) else if /I "%MODE%"=="server" (
    cmake .. -DBUILD_CLIENT=OFF -DBUILD_SERVER=ON -DBUILD_LOCAL=OFF -DCMAKE_TOOLCHAIN_FILE=C:/Users/Lakshay/vcpkg/scripts/buildsystems/vcpkg.cmake
) else if /I "%MODE%"=="local" (
    cmake .. -DBUILD_LOCAL=ON -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF -DCMAKE_TOOLCHAIN_FILE=C:/Users/Lakshay/vcpkg/scripts/buildsystems/vcpkg.cmake
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
    if exist build\%CONFIG%\GlCraftClient.exe (
        build\%CONFIG%\GlCraftClient.exe
    )
) else if /I "%MODE%"=="local" (
    if exist build\%CONFIG%\GlCraftLocal.exe (
        build\%CONFIG%\GlCraftLocal.exe
    )
)

goto :eof

:error
echo An error occurred.
pause
