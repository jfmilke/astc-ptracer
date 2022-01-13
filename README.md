# Combined ASTC Particle Tracer
A pipeline framework to compress a 3D/4D vector field with ASTC, integrate it and render the results.
Result of my Bachelor's thesis to examine the doability and performance of (ASTC) texture compression on vector fields before integration.
Build on top of a [minimalistic renderer](https://devhub.vr.rwth-aachen.de/VR-Group/jay).

Takes as input a vector field in netCDF/HDF5 file format or plain data.

### Compatibility
Tested under Windows 10, not tested for Linux.
Needs a ASTC compatible GPU, which restricts to use Intel HD Graphics for regular computer hardware.

### Requirements
- CMake 3.15+
- Git 2.24+
- GCC 9+ or Visual Studio 2019+

### Dependencies
- boost
- catch2
- glfw3
- globjects
- highfive
- astc-encoder
- anttweakbar

## Building:
- Clone the repository.
- Run `bootstrap.[sh|bat]`. It takes approximately 20 minutes to download, build and install all dependencies.
- The binaries are then available under the `./build` folder.

## Sample Files:
- A vector field in HDF5 format can be obtained from https://cgl.ethz.ch/research/visualization/data.php
    - Library has been tested with Cloud-topped Boundary Layer, Half Cylinder Ensemble & Research Vessel Tangaroa

## Problems:
- The vcpkg-build could fail because of hdf5 + highfive
    - Be sure that the highfive-port from the utility-folder is used
    - Try uninstalling hdf5 + highfive via "vcpkg remove hdf5 --recurse" and then perform "vcpkg install highfive:x64-windows"