# Simple Conan

This follows the tutorial in:
<https://docs.conan.io/2/tutorial/consuming_packages/build_simple_cmake_project.html>

There where a number of changes due to different build environment and updates to conan itself.

## Conan Profile

To create the profile I ran:

```bash
conan profile detect --force
```

It detected msvc, so I change the profile to my gcc install. This is the default conan profile in:
`~\.conan2\profiles`

```
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=20
compiler.version=11.3
compiler.libcxx=libstdc++11
os=Windows
```

## Conan with Ninja

In `~\.conan2\global.conf`, I added the following line to make conan generate cmake files that call ninja rather
than make.

```
tools.cmake.cmaketoolchain:generator = Ninja
```

## Building Application

```bash
conan install . --output-folder=build --build=missing
cmake --preset conan-release
cd build
ninja
```