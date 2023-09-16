
#include <chrono>
#include <cstdint>
#include <execution>
#include <iostream>
#include <future>
#include <thread>
#include <vector>

#include "joiner.h"

template <typename Iterator, typename Func>
void parallelForEachUsingPackageTask(Iterator first, Iterator last, Func f) {

    size_t length = std::distance(first, last);
    if (!length) {
        return;
    }

    // Calculate the optimized number of threads to run the algorithm on
    const size_t minPerThread = 25;
    const size_t maxThreads = (length + minPerThread - 1) / minPerThread;
    const size_t hardwareThreads = std::thread::hardware_concurrency();
    const size_t numThreads = std::min(hardwareThreads != 0 ? hardwareThreads : 2, maxThreads);
    const size_t blockSize = length / numThreads;

    // Declare the needed data structures
    std::vector<std::future<void>> futures(numThreads - 1);
    std::vector<std::thread> threads(numThreads - 1);
    Joiner joiner(threads);

    Iterator blockStart = first;
    for (size_t i = 0; i <= numThreads - 2; i++) {
        Iterator blockEnd = blockStart;
        std::advance(blockEnd, blockSize);
        std::packaged_task<void(void)> task([=]() {
            std::for_each(blockStart, blockEnd, f);
        });
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task));
        blockStart = blockEnd;
    }

    // Call the function for last block from this thread
    std::for_each(blockStart, last, f);

    // Wait until futures are ready
    for (auto &f : futures) {
        f.get();
    }
}

template <typename Iterator, typename Func>
void parallelForEachUsingAsync(Iterator first, Iterator last, Func f) {

    size_t length = std::distance(first, last);
    if (!length) {
        return;
    }

    size_t minPerThread = 25;
    if (length < 2 * minPerThread) {
        std::for_each(first, last, f);
    } else {
        const Iterator midPoint = first + length / 2;
        std::future<void> firstHalf = std::async(&parallelForEachUsingAsync<Iterator, Func>, first, midPoint, f);
        parallelForEachUsingAsync(midPoint, last, f);
        firstHalf.get();
    }
}


int main() {

    static constexpr size_t testSize = 1000;
    std::vector<int> ints(testSize);
    std::fill(ints.begin(), ints.end(), 1);

    auto longFunction = [](const int &n) { 
        int sum = 0;
        for (auto i = 0; i < 100000; i++) {
            sum += 1 * (i - 499);
        }
    };
    
    auto startTime = std::chrono::high_resolution_clock::now();
    std::for_each(ints.cbegin(), ints.cend(), longFunction);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "1) " << dur << std::endl;

    startTime = std::chrono::high_resolution_clock::now();
    std::for_each(std::execution::seq, ints.cbegin(), ints.cend(), longFunction);
    endTime = std::chrono::high_resolution_clock::now();
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "2) " << dur << std::endl;

    startTime = std::chrono::high_resolution_clock::now();
    std::for_each(std::execution::par, ints.cbegin(), ints.cend(), longFunction);
    endTime = std::chrono::high_resolution_clock::now();
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "3) " << dur << std::endl;

    startTime = std::chrono::high_resolution_clock::now();
    parallelForEachUsingPackageTask(ints.cbegin(), ints.cend(), longFunction);
    endTime = std::chrono::high_resolution_clock::now();
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "4) " << dur << std::endl;

    startTime = std::chrono::high_resolution_clock::now();
    parallelForEachUsingAsync(ints.cbegin(), ints.cend(), longFunction);
    endTime = std::chrono::high_resolution_clock::now();
    dur = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "5) " << dur << std::endl;
}