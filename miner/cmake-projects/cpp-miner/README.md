# CPP Miner

A simple C++ Bitcoin miner.

## Build Instructions

```bash
cmake --preset debug
cd build-debug
ninja
```

# clang-tidy

```bash
clang-tidy -p ./build-debug src/main.cpp
```

## References

* [Using CMake and managing dependencies](https://edw.is/using-cmake/)
* [Descriptions of SHA-256, SHA-384, and SHA-512](https://eips.ethereum.org/assets/eip-2680/sha256-384-512.pdf)
* [Block Chain](https://developer.bitcoin.org/reference/block_chain.html)
* [Block hashing algorithm](https://en.bitcoin.it/wiki/Block_hashing_algorithm)
