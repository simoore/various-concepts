# Simple GTest

This is an example of running gtest using conan to download and build the testing framework.

To get the MinGW compiled app to print to console, I needed to add the following dependancies:

```cmake
target_link_options(simple-gtest PRIVATE -static-libasan -static-libgcc -static-libstdc++)
```

## Search Include Paths

At the moment I'm hard coding include paths in the `c_cpp_properties.json` file. There may be better ways to
do this.