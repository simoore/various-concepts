#pragma once

#include <cstdint>

#include "miner/block.h"
#include "miner/sha256.h"

namespace miner {

class Miner {
public:

    ///////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    ///////////////////////////////////////////////////////////////////////////

    static bool validHash(const BlockHeader::Threshold &threshold, const Sha256::HashValue &hashValue);

    static uint32_t mine(const uint32_t startNonce, BlockHeader &header);
};

}