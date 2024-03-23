#include <iostream>
#include <vector>

#include "gtest/gtest.h"

#include "miner/miner.h"

using namespace ::testing;

class TestFixture_Miner : public Test {
protected:
};

TEST_F(TestFixture_Miner, TestValidHash) {
    miner::BlockHeader::Threshold t1 = {0x9, 0, 0, 0, 0, 0, 0, 0};
    miner::Sha256::HashValue h1 = {0x8, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_TRUE(miner::Miner::validHash(t1, h1));

    miner::BlockHeader::Threshold t2 = {0x9, 0x1, 0, 0, 0, 0, 0, 0};
    miner::Sha256::HashValue h2 = {0xAA, 0, 0, 0, 0, 0, 0, 0};
    ASSERT_TRUE(miner::Miner::validHash(t2, h2));

    miner::BlockHeader::Threshold t3 = {0x9, 0x1, 0, 0, 0, 0, 0, 0};
    miner::Sha256::HashValue h3 = {0x9, 0x1, 0, 0, 0, 0, 0, 0};
    ASSERT_FALSE(miner::Miner::validHash(t3, h3));
}
