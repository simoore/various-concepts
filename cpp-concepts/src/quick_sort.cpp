#include <algorithm>
#include <future>
#include <iostream>
#include <list>

template <typename T>
std::list<T> sequentialQuickSort(std::list<T> input) {

    // recursive return condition
    if (input.size() < 2) {
        return input;
    }

    // select a pivot value
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T pivot = *result.begin();

    // arrange the input array
    auto dividePoint = std::partition(input.begin(), input.end(), [&](const T &t) {
        return t < pivot;
    });

    // call the sequentialQuickSort recursively
    std::list<T> lowerList;
    lowerList.splice(lowerList.end(), input, input.begin(), dividePoint);

    auto newLower(sequentialQuickSort(std::move(lowerList)));
    auto newUpper(sequentialQuickSort(std::move(input)));

    // arraning the result list
    result.splice(result.begin(), newLower);
    result.splice(result.end(), newUpper);

    return result;
}

template <typename T>
std::list<T> parallelQuickSort(std::list<T> input) {

    // recursive return condition
    if (input.size() < 2) {
        return input;
    }

    // select a pivot value
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T pivot = *result.begin();

    // arrange the input array
    auto dividePoint = std::partition(input.begin(), input.end(), [&](const T &t) {
        return t < pivot;
    });

    // call the sequentialQuickSort recursively
    std::list<T> lowerList;
    lowerList.splice(lowerList.end(), input, input.begin(), dividePoint);

    auto newLower(sequentialQuickSort(std::move(lowerList)));
    std::future<std::list<T>> newUpperFuture(std::async(parallelQuickSort<T>, std::move(input)));
    //auto newUpper(sequentialQuickSort(std::move(input)));

    // arraning the result list
    result.splice(result.begin(), newLower);
    result.splice(result.end(), newUpperFuture.get());

    return result;
}

int main() {
    std::list<int> unsorted{6, 8, 2, 9, 1, 0, 5, 3, 7, 4};
    auto list1 = sequentialQuickSort(unsorted);
    for (auto e : list1) {
        std::cout << e;
    }
    std::cout << std::endl;

    std::list<int> unsorted2{6, 8, 2, 9, 1, 0, 5, 3, 7, 4};
    auto list2 = parallelQuickSort(unsorted2);
    for (auto e : list2) {
        std::cout << e;
    }
    std::cout << std::endl;
}