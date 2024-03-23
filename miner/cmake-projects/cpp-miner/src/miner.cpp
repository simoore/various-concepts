#include <algorithm>
#include <bit>
#include <chrono>
#include <iostream>
#include <string>

#include "miner/miner.h"

namespace miner {

/// The hash needs to be less than the threshold to be a valid hash.
bool Miner::validHash(const BlockHeader::Threshold &threshold, const Sha256::HashValue &hashValue) {
    const auto lhs = std::ranges::reverse_view(hashValue);
    const auto rhs = std::ranges::reverse_view(threshold);
    return std::ranges::lexicographical_compare(lhs, rhs);
}

uint32_t Miner::mine(const uint32_t startNonce, BlockHeader &header) {

    auto threshold = BlockHeader::nbitsToThreshold(header.nbits());
    uint32_t nonce = startNonce;
    uint32_t count = 0;

    auto start = std::chrono::steady_clock::now();

    while (true) {
        
        header.setNonce(nonce);
        auto hashValue = Sha256::hash(header.data());
        hashValue = Sha256::hash(hashValue);
        
        if (validHash(threshold, hashValue)) {
            return nonce;
        }
        
        if (count == 10000) {
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            double hashrate = 10000000.0 / static_cast<double>(duration);
            std::cout << "Currently mining at " << hashrate << " hashes / second" << std::endl;
            start = std::chrono::steady_clock::now();
            count = 0;
        }

        nonce++;
        count++;
    }
}

} // namespace miner