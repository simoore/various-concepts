#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <span>
#include <tuple>

namespace miner {

class Sha256 {
public:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS AND TYPES
    ///////////////////////////////////////////////////////////////////////////

    // I have only tested on a little-endian processor. This is here to remind you to test on a big endian machine.
    static_assert(std::endian::native == std::endian::little);

    static constexpr uint32_t sHashSize = 8;
    static constexpr uint32_t sBlockSize = 16;
    static constexpr uint32_t sFirstPadWord = 0x80000000;

    using HashValue = std::array<uint32_t, sHashSize>;
    using Block = std::array<uint32_t, sBlockSize>;
    using BigBlock = std::array<uint32_t, 64>;

    static constexpr HashValue sInitialHash = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    static constexpr BigBlock sK = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    template <size_t... indices>
    static void unroll (auto f, std::index_sequence <indices...>) {
        (f.template operator () <indices> (), ...);
    }

    /// Swaps the host computer endianess to big endian. 
    static uint32_t tobe(uint32_t x) {
        if constexpr (std::endian::native == std::endian::little) {
            return std::byteswap(x);
        } else {
            return x;
        }
    }

    static std::tuple<uint32_t, uint32_t> length2Words(uint64_t l) {
        return std::make_tuple(tobe(static_cast<uint32_t>(l >> 32)), tobe(static_cast<uint32_t>(l & 0xFFFFFFFF)));
    }

    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }

    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static uint32_t bigSig0(uint32_t x) {
        return std::rotr(x, 2) ^ std::rotr(x, 13) ^ std::rotr(x, 22);
    }

    static uint32_t bigSig1(uint32_t x) {
        return std::rotr(x, 6) ^ std::rotr(x, 11) ^ std::rotr(x, 25);
    }

    static uint32_t smallSig0(uint32_t x) {
        return std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
    }

    static uint32_t smallSig1(uint32_t x) {
        return std::rotr(x, 17) ^ std::rotr(x, 19) ^ (x >> 10);
    }

    static uint32_t calcT1(uint32_t e, uint32_t f, uint32_t g, uint32_t h, uint32_t k, uint32_t w) {
        return h + bigSig1(e) + ch(e, f, g) + k + w;
    }

    static HashValue hashBlock(const HashValue &lastHash, const Block &block);

    static HashValue hash(std::span<const uint32_t> dataSpan);

    static void printHash(const HashValue &hash);
};

} // namespace miner