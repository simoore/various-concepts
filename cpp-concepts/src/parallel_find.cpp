#include <atomic>
#include <chrono>
#include <execution>
#include <future>
#include <iostream>
#include <vector>

std::atomic<bool> done{false};

template <typename iterator, typename MatchType>
iterator parallelFind(iterator first, iterator last, MatchType match) {
    // The performance of this is atrocious compared to the STL versions
    const size_t length = std::distance(first, last);
    const size_t minPerThread = 10000;
    if (length < minPerThread) {
        for (; (first != last) && done.load(); ++first) {
            if (*first == match) {
                done.store(true);
                return first;
            }
        }
        return last;
    } else {
        const iterator midPoint = first + length / 2;
        std::future<iterator> asyncResult = 
            std::async(parallelFind<iterator, MatchType>, midPoint, last, match);
        const iterator directResult = parallelFind(first, midPoint, match);
        return (directResult == midPoint) ? asyncResult.get() : directResult;
    }
}

int main() {

    static constexpr size_t testSize = 100000000;
    std::vector<int> ints(testSize);
	for (size_t i = 0; i < testSize; i++) {
		ints[i] = i;
	}

	int lookingFor = 50000000;

	auto startTime = std::chrono::high_resolution_clock::now();
	parallelFind(ints.begin(), ints.end(), lookingFor);
	auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "1) " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime) << std::endl;

	startTime = std::chrono::high_resolution_clock::now();
	std::find(ints.begin(), ints.end(), lookingFor);
	endTime = std::chrono::high_resolution_clock::now();
	std::cout << "2) " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime) << std::endl;

	startTime = std::chrono::high_resolution_clock::now();
	std::find(std::execution::par,ints.begin(), ints.end(), lookingFor);
	endTime = std::chrono::high_resolution_clock::now();
	std::cout << "3) " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime) << std::endl;

	startTime = std::chrono::high_resolution_clock::now();
	std::find(std::execution::seq, ints.begin(), ints.end(), lookingFor);
	endTime = std::chrono::high_resolution_clock::now();
	std::cout << "4) " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime) << std::endl;

	return 0;
}