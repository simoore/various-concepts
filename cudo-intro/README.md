# CUDA Intro

## Visual Studio Build Tools

I've installed visual studio build tools 2022. Next add the developer command prompt to the list of available terminals 
in vscode using the following workspace settings:

```json
{
    "terminal.integrated.defaultProfile.windows": "VSDev",
    "terminal.integrated.profiles.windows": {
        "VSDev": {
            "label": "VSDev",
            "path": "cmd.exe",
            "args": [
                "/k",
                "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat"
            ],
            "env": {}
        }
    }
}
```

Then you run cmake in the developer terminal to make it use the MSVC compiler. For example:

```bat
cmake --preset debug
cd build-debug
ninja
```

There are a number of environmental variable .bat files to chose from. The environmental variable script I use was 
chosen because it sets the path to the x64 version of cl not the x86 version. You'll have numerous compile errors
if you use the wrong version of the MSVC compiler.

## CUDA Environment

Download a version of CUDA that supports your graphics card. See: <https://en.wikipedia.org/wiki/CUDA#GPUs_supported>. 
Once it is installed, you can check to Nvidia Cuda compiler is install on the path:

```bat
nvcc --version
```

I didn't install any visual studio integration on profiling tools.

## CUDA Hello World

Compile the `hello.cu` file using:

```bat
nvcc src/hello.cu -o hello
```
