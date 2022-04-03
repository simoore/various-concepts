# Soundex Test

On the first run, you need to specify the type of build system cmake is using.
And then execute cmake on the build directory.

```
mkdir build
cd build
cmake .. -G "Unix Makefiles"
make
```

After the first run, you can rebuild the makefile without the `-G` option.
