# Nanobp

```bash
# Install the protoc compiler using pip.
pip install protobuf grpcio-tools

# Create the cmake build configuration.
cmake --preset=debug
cd build-debug

# Build application.
ninja
```

## Manual Protobuf Code Generation

```bash
cd ../proto
python ../build-debug/_deps/nanopb-src/generator/nanopb_generator.py messages.proto
```

## References

[Nanopb: Basic concepts](https://jpa.kapsi.fi/nanopb/docs/concepts.html#encoding-callbacks)
[nanopb server-client example ](https://gist.github.com/nolanholden/ff1e4ec544dc9a225f9289cd3c563bf4)