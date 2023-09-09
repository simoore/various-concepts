// https://hannes.hauswedell.net/post/2019/11/30/range_intro/

#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>


// STL method to copy only even numbers, then skips the first number then reverses all the elements.
void algorithmA() {
    const std::vector numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto even = [](int i){ return 0 == i % 2; };

    std::vector<int> temp;
    std::copy_if(std::begin(numbers), std::end(numbers), std::back_inserter(temp), even);
    std::vector<int> temp2(std::begin(temp) + 1, std::end(temp));

    for (auto iter = std::rbegin(temp2); iter != std::rend(temp2); ++iter) {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;
}

// AlgorithmA, but with ranges
void algorithmB() {
    const std::vector numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto even = [](int i){ return 0 == i % 2; };

    auto ans = numbers | std::views::filter(even) | std::views::drop(1) | std::views::reverse;
   
    for (auto element : ans) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

// This filters out odd numbers, then multiplies the results by 2.
void algorithmC() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
  
    auto results = numbers 
        | std::views::filter([](int n){ return n % 2 == 0; })
        | std::views::transform([](int n){ return n * 2; });
                           
    for (auto v: results) std::cout << v << " ";     // 4 8 12
    std::cout << std::endl;
}

// Placeholder types for functions is a C++20 features. It is basically a template parameter without having to 
// write the template explicitly.
void printRange(auto range) {
    for (auto r : range) {
        std::cout << r << " ";
    }
    std::cout << std::endl;
}

void examples() {
    std::cout << "## Examples" << std::endl;
    auto numbers = std::views::iota(1, 10);
    printRange(numbers);
    printRange(std::views::drop(numbers, 5));
    printRange(std::views::reverse(numbers));

    std::vector vec{1, 2, 3, 4, 5, 6};
    auto even = [](int i){return i % 2 == 0; };
    auto square = [](int i){return i*i; };
    auto v = vec | std::views::filter(even) | std::views::transform(square);
    printRange(v);
}

int main() {
    algorithmA();
    algorithmB();
    algorithmC();
    examples();
}

