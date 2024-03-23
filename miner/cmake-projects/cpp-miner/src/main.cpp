#include <iostream>

#include "miner/block.h"
#include "miner/miner.h"

static constexpr uint32_t sStartNonce = 2080000000UL;

int main() {

    miner::BlockHeader header = miner::BlockHeader::genesisBlock();

    auto nonce = miner::Miner::mine(sStartNonce, header);
    
    header.setNonce(nonce);
    auto hashValue = miner::Sha256::hash(miner::Sha256::hash(header.data()));

    std::cout << "Block solved ! Nonce: " << nonce << '\n';
    miner::Sha256::printHash(hashValue);
    
    return 0;
}
