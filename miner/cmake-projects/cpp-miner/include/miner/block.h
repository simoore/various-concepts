#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <ranges>
#include <span>
#include <string>
#include <vector>

namespace miner {

class BlockHeader {
public:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC CONSTANTS
    ///////////////////////////////////////////////////////////////////////////

    static constexpr size_t sHeaderSize = 20;
    static constexpr size_t sThresholdSize = 8;
    static constexpr size_t sHashSize = 8;
    static constexpr size_t sNibblesPerWord = 8;
    static constexpr size_t sBitsPerByte = 8;
    static constexpr size_t sBitsPerNibble = 4;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC TYPES
    ///////////////////////////////////////////////////////////////////////////

    using Threshold = std::array<uint32_t, sThresholdSize>;

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    BlockHeader(const std::string &version, const std::string &prevhash, const std::string &merkleRoot, 
        const std::string &time, const std::string &nbits);

    uint32_t version(void) const {
        return mBlockHeader[0];
    }

    std::span<const uint32_t> prevhash(void) const {
        return std::span<const uint32_t>(mBlockHeader + 1, 8);
    }

    std::span<const uint32_t> merkleRoot(void) const {
        return std::span<const uint32_t>(mBlockHeader + 9, 8);
    }

    uint32_t time(void) const {
        return mBlockHeader[17];
    }

    uint32_t nbits(void) const {
        return mBlockHeader[18];
    }

    uint32_t nonce(void) const {
        return mBlockHeader[19];
    }

    void setNonce(uint32_t nonce) {
        mBlockHeader[19] = nonce;
    }

    std::span<uint32_t> data(void) {
        return std::span<uint32_t>(mBlockHeader, sHeaderSize);
    }

    static Threshold nbitsToThreshold(uint32_t nbits);

    static std::vector<uint32_t> hexStrToBinary(const std::string &hex);

    static BlockHeader genesisBlock(void);

private:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FIELDS
    ///////////////////////////////////////////////////////////////////////////

    uint32_t mBlockHeader[sHeaderSize]; 

};

}
