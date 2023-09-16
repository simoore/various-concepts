#pragma once

#include <thread>
#include <vector>

class Joiner {
public:

    explicit Joiner(std::vector<std::thread> &threads): mThreads(threads) {}

    ~Joiner() {
        for (auto &t : mThreads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:

    std::vector<std::thread> &mThreads;
};