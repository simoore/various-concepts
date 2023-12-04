# Tool Requires

This follows the tutorial from:
<https://docs.conan.io/2/tutorial/consuming_packages/use_tools_as_conan_packages.html>

The environmental variables worked in cmd not powershell

```bash
conan install . --output-folder=build --build=missing
cd build
conanbuild.bat
cmake <path> -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:\doc\dev\miner\cmake-projects\tool-requires\build\conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_BUILD_TYPE=Release
```