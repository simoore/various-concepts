#include <iostream>
#include <set>
#include <type_traits>
#include <vector>

// Requires keyword applies a 
template <typename T>
requires (!std::is_pointer_v<T>)
T maxValue1(T a, T b) {
    return b < a ? a : b;
}

// Concept is a templated expression that evaluates to bool at compile time.
template <typename T>
concept IsSimplePointer = std::is_pointer_v<T>;

// The above concept works only if the type is a pointer but not if operator* is defined, such as with smart pointers.
// We can use the following syntax to ensure all given expressions are valid.
template <typename T>
concept IsPointer = requires(T p) { // This is a requires expression. You can use this directly rather than assign
                                    // it to a concept as well.
    *p;     // *p must be valid for type T
    {p < p} -> std::convertible_to<bool>; // < yields bool
    p == nullptr; // can compare with nullptr (this rules out iterators)
};

template <typename T>
requires (!IsPointer<T>)    // If we leave this out, this becomes the primary version of this templated function
                            // Adn the version below is a more specialized version.
T maxValue2(T a, T b) {
    return b < a ? a : b;
}

// We can provide a max value for pointer version by ensuring at most one implementation exists per set of template
// parameters.
template <typename T>
requires IsPointer<T>
auto maxValue2(T a, T b) {
    return maxValue2(*a, *b); 
}

// If the constraint is a concept applied to a single parameter, it can be used in lieu of the typename keyword
template <IsPointer T>
auto maxValue3(T a, T b) {
    return maxValue2(*a, *b); 
}

// Also, type constraints can be used directly in the functionbn parameters. The one problem with this is that now
// the two types can have different types, so the more traditional declaration above ensures they have the same type.
auto maxValue4(const IsPointer auto &a, const IsPointer auto &b) {
    return maxValue2(*a, *b);
}

// But there is another way to express constraints on the type above while using the parameter placeholder tpye syntax.
auto maxValue5(const IsPointer auto &a, const IsPointer auto &b) 
requires std::is_same_v<decltype(*a), decltype(*b)> {
    return maxValue2(*a, *b);
}

// You can use requires expressions in constexpr. Like this generic insert function for STL containers that use 
// insert() or push_back().
void addElem(auto container, auto value) {
    if constexpr (requires { container.push_back(value); }) {
        container.push_back(value);
    } else {
        container.insert(value);
    }
}

// Upto 3.3 in C++20 The Complete Guide

int main() {
    int x = 42;
    int y = 77;
    std::cout << maxValue1(x, y) << std::endl;
    // std::cout << maxValue1(&x, &y) << '\n'; // ERROR: constraint not met
    std::cout << maxValue2(x, y) << std::endl;
    std::cout << maxValue2(&x, &y) << std::endl;
    std::cout << maxValue3(&x, &y) << std::endl;
    std::cout << maxValue4(&x, &y) << std::endl;
    std::cout << maxValue5(&x, &y) << std::endl;

    std::vector<int> v;
    std::set<int> s;
    addElem(v, 2);
    addElem(s, 4);
}