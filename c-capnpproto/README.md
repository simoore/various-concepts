# C-Capnp

```bash
# In msys install capnp
pacman -S mingw-w64-ucrt-x86_64-capnproto

# Create the cmake build configuration.
cmake --preset=debug
cd build-debug

# Build application.
ninja
```

I'm getting this error while trying to parse the example addressbook schema:

```bash
failed to read schema from stdin
``````