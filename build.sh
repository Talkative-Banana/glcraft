#!/bin/bash
set -e

MODE=${1:-client}

mkdir -p build
cd build

case $MODE in
client)
    cmake -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF -DBUILD_LOCAL=OFF ..
    ;;
server)
    cmake -DBUILD_CLIENT=OFF -DBUILD_SERVER=ON -DBUILD_LOCAL=OFF ..
    ;;
local)
    cmake -DBUILD_LOCAL=ON -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF ..
    ;;
*)
    echo "Usage: ./build.sh [client|server|local]"
    exit 1
    ;;
esac

if command -v nproc >/dev/null 2>&1; then
    BUILD_JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    BUILD_JOBS=$(sysctl -n hw.ncpu)
else
    BUILD_JOBS=2
fi
cmake --build . --parallel "$BUILD_JOBS"
cd ..
./GlCraft${MODE^}
