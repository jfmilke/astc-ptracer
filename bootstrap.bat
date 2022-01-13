

if not exist "build" mkdir build
cd build
if not exist "vcpkg" git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
if not exist "vcpkg.exe" call bootstrap-vcpkg.bat

set VCPKG_DEFAULT_TRIPLET=x64-windows
rem Add your library ports here.
rem stb is provided as an cursed entity part of astc-encoder pls dont judge
vcpkg install --recurse --overlay-ports=../../utility/overlay_ports boost-multi-array boost-odeint boost-ublas catch2 glfw3 globjects highfive astc-encoder anttweakbar
cd ..

cmake -Ax64 -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --target ALL_BUILD --config Release
cd ..   