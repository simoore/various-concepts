#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <random>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parallel Sort
// -------------
// Provide an execution policy to sort to allow STL algorithms to be executed in parallel.
// I feel I'm missing a library because the application the sorting takes the same amount of time regarless of the
// execution policy.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace parallelsort {

void printResults(const char *tag, const std::vector<double> &sorted, 
    std::chrono::high_resolution_clock::time_point startTime, 
    std::chrono::high_resolution_clock::time_point endTime) {
    using namespace std::chrono;
    auto dur = duration_cast<duration<double, std::milli>>(endTime - startTime).count();
    std::cout << tag << ": Lowest: " << sorted.front() << " Highest: " << sorted.back() << " Time: " 
        << dur << std::endl;
}

void run(void) {

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "-- PARALLEL SORT" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    std::random_device rd;

    static constexpr int testSize = 10'000'000;
    static constexpr int iterationCount = 5;

    std::cout << "Testing with " << testSize << " doubles..." << std::endl;
    std::vector<double> doubles(testSize);
    for (auto &d : doubles) {
        d = static_cast<double>(rd());
    }

    for (int i = 0; i < iterationCount; ++i) {
        std::vector<double> sorted(doubles);
        const auto startTime = std::chrono::high_resolution_clock::now();
        std::sort(sorted.begin(), sorted.end());
        const auto endTime = std::chrono::high_resolution_clock::now();
        printResults("Sequential STL", sorted, startTime, endTime);
    }

    for (int i = 0; i < iterationCount; ++i) {
        std::vector<double> sorted(doubles);
        const auto startTime = std::chrono::high_resolution_clock::now();
        std::sort(std::execution::par_unseq, sorted.begin(), sorted.end());
        const auto endTime = std::chrono::high_resolution_clock::now();
        printResults("Parallel STL", sorted, startTime, endTime);
    }
}

}

int main() {
    parallelsort::run();
}