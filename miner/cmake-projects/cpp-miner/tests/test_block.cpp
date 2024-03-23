#include <bit>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"

#include "miner/block.h"

using namespace ::testing;

class TestFixture_Block : public Test {
protected:
};

TEST_F(TestFixture_Block, TestHexStrToBinary) {
    auto binary = miner::BlockHeader::hexStrToBinary("0123456789ABCDEF");
    ASSERT_EQ(binary.size(), 2);
    ASSERT_EQ(binary[0], 0x67452301);
    ASSERT_EQ(binary[1], 0xEFCDAB89);

    binary = miner::BlockHeader::hexStrToBinary("0000000400000002");
    ASSERT_EQ(binary.size(), 2);
    ASSERT_EQ(binary[0], 0x04000000);
    ASSERT_EQ(binary[1], 0x02000000);

    binary = miner::BlockHeader::hexStrToBinary("D3421A423F980");
    ASSERT_EQ(binary.size(), 2);
    ASSERT_EQ(binary[0], 0x421A42D3);
    ASSERT_EQ(binary[1], 0x0983F);
}

TEST_F(TestFixture_Block, TestBlockCreation) {
    const std::string version = "01000000";
    const std::string prevhash = "0000000000000000000000000000000000000000000000000000000000000000";
    const std::string merkleRoot = "3BA3EDFD7A7B12B27AC72C3E67768F617FC81BC3888A51323A9FB8AA4B1E5E4A";
    const std::string time = "29AB5F49";
    const std::string nbits = "FFFF001D";

    auto block = miner::BlockHeader(version, prevhash, merkleRoot, time, nbits);
    block.setNonce(0x43456534);

    std::vector<uint32_t> expected = {
        0x00000001, 
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        std::byteswap(0x3BA3EDFDU), std::byteswap(0x7A7B12B2U), std::byteswap(0x7AC72C3EU), std::byteswap(0x67768F61U),
        std::byteswap(0x7FC81BC3U), std::byteswap(0x888A5132U), std::byteswap(0x3A9FB8AAU), std::byteswap(0x4B1E5E4AU), 
        std::byteswap(0x29AB5F49U), std::byteswap(0xFFFF001DU),
        0x43456534
    };
    std::vector<uint32_t> actual = std::vector<uint32_t>{block.data().begin(), block.data().end()};
    ASSERT_EQ(actual, expected);
}

TEST_F(TestFixture_Block, TestNBitsToDifficulty) {
    auto t1 = miner::BlockHeader::nbitsToThreshold(0x01003456);
    miner::BlockHeader::Threshold e1 = {0, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_EQ(t1, e1);

    auto t2 = miner::BlockHeader::nbitsToThreshold(0x01123456);
    miner::BlockHeader::Threshold e2 = {0x12, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_EQ(t2, e2);

    auto t3 = miner::BlockHeader::nbitsToThreshold(0x02008000);
    miner::BlockHeader::Threshold e3 = {0x80, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_EQ(t3, e3);

    auto t4 = miner::BlockHeader::nbitsToThreshold(0x05009234);
    miner::BlockHeader::Threshold e4 = {0x92340000, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_EQ(t4, e4);

    auto t5 = miner::BlockHeader::nbitsToThreshold(0x04123456);
    miner::BlockHeader::Threshold e5 = {0x12345600, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_EQ(t5, e5);

    auto t6 = miner::BlockHeader::nbitsToThreshold(0x181bc330);
    miner::BlockHeader::Threshold e6 = {0, 0, 0, 0, 0, 0x1bc33000, 0, 0};
    ASSERT_EQ(t6, e6);
}

TEST_F(TestFixture_Block, TestGenesisBlockContentCorrect) {
    // https://en.bitcoin.it/wiki/Genesis_block
    auto header = miner::BlockHeader::genesisBlock();

    std::vector<uint32_t> expectedPrevHash{{0, 0, 0, 0, 0, 0, 0, 0}};
    std::vector<uint32_t> actualPrevHash{header.prevhash().begin(), header.prevhash().end()};

    std::vector<uint32_t> expectedMerkleRoot{{0xfdeda33b, 0xb2127b7a, 0x3e2cc77a, 0x618f7667, 0xc31bc87f, 
        0x32518a88, 0xaab89f3a, 0x4a5e1e4b}};
    std::vector<uint32_t> actualMerkleRoot{header.merkleRoot().begin(), header.merkleRoot().end()};

    ASSERT_EQ(header.version(), 1);
    ASSERT_EQ(expectedPrevHash, actualPrevHash);
    ASSERT_EQ(expectedMerkleRoot, actualMerkleRoot);
    ASSERT_EQ(header.time(), 0x495FAB29);
    ASSERT_EQ(header.nbits(), 0x1d00ffff);
    ASSERT_EQ(header.nonce(), 2083236893U);
    ASSERT_EQ(header.nonce(), 0x7C2BAC1D);
}