#!/bin/bash

if [ ! -d "build" ]; then mkdir build; fi
cd build
if [ ! -d "vcpkg" ]; then git clone https://github.com/Microsoft/vcpkg.git; fi
cd vcpkg
if [ ! -f "vcpkg" ]; then ./bootstrap-vcpkg.sh; fi

VCPKG_DEFAULT_TRIPLET=x64-linux
# Add your library ports here. 
# stb is provided as an cursed entity part of astc-encoder pls dont judge
# Also, if installation fails, its probably because of astc-encoder. If thats the case the cmakelists.txt of astc-encoder must be reworked for the linux part (pls no)
vcpkg install --recurse --overlay-ports=../../utility/overlay_ports boost-multi-array boost-odeint boost-ublas catch2 glfw3 globjects highfive astc-encoder anttweakbar
cd ..

cmake -Ax64 -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --target ALL_BUILD --config Release
cd ..