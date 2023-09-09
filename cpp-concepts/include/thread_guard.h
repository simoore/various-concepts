#pragma once

#include <thread>

class ThreadGuard {

    std::thread &mThread;

public:

    explicit ThreadGuard(std::thread &t) : mThread(t) {}

    ~ThreadGuard() {
        if (mThread.joinable()) {
            mThread.join();
        }
    }

    // No copying
    ThreadGuard(const ThreadGuard &) = delete;
    ThreadGuard &operator=(const ThreadGuard &) = delete;

};