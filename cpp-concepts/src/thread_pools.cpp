#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:

    ThreadPool() {
        const auto threadCount = std::thread::hardware_concurrency();
        for (size_t i = 0; i < threadCount; i++) {
            mThreads.emplace_back(&ThreadPool::workerThread, this);
        }
    }

    ~ThreadPool() {
        mDone.store(true);
    }

    void workerThread(void) {

        std::function<void()> task;
        auto tryToGetTask = [&]() -> bool {
            std::lock_guard lock{mMutex};
            if (!mQueue.empty()) {
                task = mQueue.front();
                mQueue.pop();
                return true;
            }
            return false;
        };

        while (!mDone.load()) {
            if (tryToGetTask()) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

    template <typename F>
    void submit(F f) {
        std::lock_guard lock(mMutex);
        mQueue.emplace(f);
    }

private:

    std::atomic<bool> mDone = false;

    std::queue<std::function<void()>> mQueue;

    std::vector<std::jthread> mThreads;

    std::mutex mMutex;
};

int main() {

    std::cout << "------------------------------" << std::endl;
    std::cout << "-- THREAD POOL" << std::endl;
    std::cout << "------------------------------" << std::endl;

    ThreadPool pool;

    for (int i = 0; i < 100; i++) {
        pool.submit([=]{
            std::cout << i << " printed by thread - " << std::this_thread::get_id() << std::endl;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

}