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

make -j$(nproc)
cd ..
./GlCraft${MODE^}
